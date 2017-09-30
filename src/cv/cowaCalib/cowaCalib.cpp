#include "cowaCalib.h"
#include "config.h"
#include <time.h>
#include <fstream>
#include <opencv2/calib3d.hpp>
#include <iomanip>

using namespace ECAT::CALIB;

void onThresholdChanged(int pos, void* data);
void onMouseCallback(int event, int x, int y, int flags, void* userdata);
const double imgArea = 640 * 480;

inline void read(const FileNode& node, Settings& x, const Settings& default_value = Settings())
{
	if (node.empty())
		x = default_value;
	else
		x.read(node);
}

inline void write(FileStorage& fs, const String&, const Settings& s)
{
	s.write(fs);
}

enum { DETECTION = 0, CAPTURING = 1, CALIBRATED = 2 };


/*----------------------------
* 功能 : 单相机标定
*----------------------------
* 函数 : RunCalibrateCamera
* 访问 : public
* 返回 : 0
*
* 参数 : inputSettingsFile       [in]    配置文件
* 参数 : outCameraDataFilePath   [out]    相机内外参数
*/
COWACALIB_API int  Cowa_Calib::RunCalibrateCamera(const string inputSettingsFile, const string outCameraDataFilePath)
{
	//! [file_read]
	Settings s;
	FileStorage fs(inputSettingsFile, FileStorage::READ); // Read the settings
	if (!fs.isOpened())
	{
		cout << "Could not open the configuration file: \"" << inputSettingsFile << "\"" << endl;
		return -1;
	}
	fs["Settings"] >> s;
	fs.release();                                         // close Settings file

	if (outCameraDataFilePath.size() > 1)
		s.outputFileName = outCameraDataFilePath;

	if (!s.goodInput)
	{
		cout << "Invalid input detected. Application stopping. " << endl;
		return -1;
	}


	vector<vector<Point2f> > imagePoints;
	Mat cameraMatrix, distCoeffs;
	Size imageSize;
	int mode = s.inputType == Settings::IMAGE_LIST ? CAPTURING : DETECTION;
	clock_t prevTimestamp = 0;
	const Scalar RED(0, 0, 255), GREEN(0, 255, 0);
	const char ESC_KEY = 27;

	//! [get_input]
	for (;;)
	{
		Mat view;
		bool blinkOutput = false;

		view = s.nextImage();
		//-----  If no more image, or got enough, then stop calibration and show result -------------
		if (mode == CAPTURING && imagePoints.size() >= (size_t)s.nrFrames)
		{
			if (runCalibrationAndSave(s, imageSize, cameraMatrix, distCoeffs, imagePoints))
				mode = CALIBRATED;
			else
				mode = DETECTION;
		}
		if (view.empty())          // If there are no more images stop the loop
		{
			// if calibration threshold was not reached yet, calibrate now
			if (mode != CALIBRATED && !imagePoints.empty())
				runCalibrationAndSave(s, imageSize, cameraMatrix, distCoeffs, imagePoints);
			break;
		}
		//! [get_input]

		imageSize = view.size();  // Format input image.
		if (s.flipVertical)    flip(view, view, 0);
		//! [find_pattern]
		vector<Point2f> pointBuf;

		bool found;

		int chessBoardFlags = CALIB_CB_FAST_CHECK | CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_NORMALIZE_IMAGE;

		if (!s.useFisheye) {
			// fast check erroneously fails with high distortions like fisheye
			//chessBoardFlags |= CALIB_CB_FAST_CHECK; // comment by wq, DO NOT USE FAST CHECK
		}

		switch (s.calibrationPattern) // Find feature points on the input format
		{
		case Settings::CHESSBOARD:
			found = findChessboardCorners(view, s.boardSize, pointBuf, chessBoardFlags);
			break;
		case Settings::CIRCLES_GRID:
			found = findCirclesGrid(view, s.boardSize, pointBuf);
			break;
		case Settings::ASYMMETRIC_CIRCLES_GRID:
			found = findCirclesGrid(view, s.boardSize, pointBuf, CALIB_CB_ASYMMETRIC_GRID);
			break;
		default:
			found = false;
			break;
		}

		if (!found) //manual click to found corner
			cout << "The single image you took does not come up to the requirements.";
		
		//! [find_pattern]
		//! [pattern_found]
		if (found)                // If done with success,
		{
			// improve the found corners' coordinate accuracy for chessboard
			if (s.calibrationPattern == Settings::CHESSBOARD)
			{
				Mat viewGray;
				cvtColor(view, viewGray, COLOR_BGR2GRAY);
				cornerSubPix(viewGray, pointBuf, Size(11, 11),
					Size(-1, -1), TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 30, 0.1));
			}

			if (mode == CAPTURING &&  // For camera only take new samples after delay time
				(!s.inputCapture.isOpened() || clock() - prevTimestamp > s.delay*1e-3*CLOCKS_PER_SEC))
			{
				imagePoints.push_back(pointBuf);
				prevTimestamp = clock();
				blinkOutput = s.inputCapture.isOpened();
			}

			// Draw the corners.
			drawChessboardCorners(view, s.boardSize, Mat(pointBuf), found);
		}
		//! [pattern_found]
		//----------------------------- Output Text ------------------------------------------------
		//! [output_text]
		string msg = (mode == CAPTURING) ? "100/100" :
			mode == CALIBRATED ? "Calibrated" : "Press 'g' to start";
		int baseLine = 0;
		Size textSize = getTextSize(msg, 1, 1, 1, &baseLine);
		Point textOrigin(view.cols - 2 * textSize.width - 10, view.rows - 2 * baseLine - 10);

		if (mode == CAPTURING)
		{
			if (s.showUndistorsed)
				msg = format("%d/%d Undist", (int)imagePoints.size(), s.nrFrames);
			else
				msg = format("%d/%d", (int)imagePoints.size(), s.nrFrames);
		}

		putText(view, msg, textOrigin, 1, 1, mode == CALIBRATED ? GREEN : RED);

		if (blinkOutput)
			bitwise_not(view, view);
		//! [output_text]
		//------------------------- Video capture  output  undistorted ------------------------------
		//! [output_undistorted]
		if (mode == CALIBRATED && s.showUndistorsed)
		{
			Mat temp = view.clone();
			undistort(temp, view, cameraMatrix, distCoeffs);
		}
		//! [output_undistorted]
		//------------------------------ Show image and check for input commands -------------------
		//! [await_input]
		imshow("Image View", view);
		char key = (char)waitKey(s.inputCapture.isOpened() ? 50 : s.delay);

		if (key == ESC_KEY)
			break;

		if (key == 'u' && mode == CALIBRATED)
			s.showUndistorsed = !s.showUndistorsed;

		if (s.inputCapture.isOpened() && key == 'g')
		{
			mode = CAPTURING;
			imagePoints.clear();
		}
		//! [await_input]
	}
	// -----------------------Show the undistorted image for the image list ------------------------
	//! [show_results]
	if (s.inputType == Settings::IMAGE_LIST && s.showUndistorsed)
	{
		Mat view, rview, map1, map2;

		Mat rview2, map1_2, map2_2;

		
		initUndistortRectifyMap(
			cameraMatrix, distCoeffs, Mat(),
			getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, imageSize, 1, imageSize, 0), imageSize,
			CV_16SC2, map1, map2);

		initUndistortRectifyMap(cameraMatrix, distCoeffs, cv::Mat::eye(3, 3, CV_64FC1), cameraMatrix, cv::Size(640, 480),
			CV_32FC1, map1_2, map2_2);
		

		for (size_t i = 0; i < s.imageList.size(); i++)
		{
			view = imread(s.imageList[i], 1);
			if (view.empty())
				continue;
			remap(view, rview, map1, map2, INTER_LINEAR);
			remap(view, rview2, map1_2, map2_2, INTER_LINEAR, BORDER_CONSTANT);
			imshow("Image Un-distort View (optimal camera matrix)", rview);
			imshow("Image Un-distort View (fix camera matrix)", rview2);
			char c = (char)waitKey();
			if (c == ESC_KEY || c == 'q' || c == 'Q')
				break;
		}
	}
	//! [show_results]

	return 0;
}

bool Cowa_Calib::runCalibration(Settings& s, Size& imageSize, Mat& cameraMatrix, Mat& distCoeffs,
	vector<vector<Point2f> > imagePoints, vector<Mat>& rvecs, vector<Mat>& tvecs,
	vector<float>& reprojErrs, double& totalAvgErr)
{
	//! [fixed_aspect]
	cameraMatrix = Mat::eye(3, 3, CV_64F);
	if (s.flag & CALIB_FIX_ASPECT_RATIO)
		cameraMatrix.at<double>(0, 0) = s.aspectRatio;
	//! [fixed_aspect]
	if (s.useFisheye) {
		distCoeffs = Mat::zeros(4, 1, CV_64F);
	}
	else {
		distCoeffs = Mat::zeros(8, 1, CV_64F);
	}

	vector<vector<Point3f> > objectPoints(1);
	calcBoardCornerPositions(s.boardSize, s.squareSize, objectPoints[0], s.calibrationPattern);

	objectPoints.resize(imagePoints.size(), objectPoints[0]);

	//Find intrinsic and extrinsic camera parameters
	double rms;
	rms = calibrateCamera(objectPoints, imagePoints, imageSize, cameraMatrix, distCoeffs, rvecs, tvecs,
			s.flag);

	// add by wq, fix K2 and K3 and re-calibrate
	std::ofstream fs_out_distort_data("out_distortion_data.txt");
	fs_out_distort_data << "original distortion is :" << distCoeffs << std::endl;
	fs_out_distort_data << "original  camera matrix is :" << cameraMatrix << std::endl;

	distCoeffs.at<double>(4, 0) = 0;
	distCoeffs.at<double>(1, 0) = 0;

	rms = calibrateCamera(objectPoints, imagePoints, imageSize, cameraMatrix, distCoeffs, rvecs, tvecs,
		s.flag | CALIB_FIX_K2 | CALIB_FIX_K3 | CALIB_USE_INTRINSIC_GUESS);

	fs_out_distort_data << "fix K2&K3 distortion is :" << distCoeffs << std::endl;
	fs_out_distort_data << "fix K2&K3  camera matrix is :" << cameraMatrix << std::endl;

	double x0 = 0 - cameraMatrix.at<double>(0, 2) / cameraMatrix.at<double>(0, 0);
	double y0 = 0 - cameraMatrix.at<double>(1, 2) / cameraMatrix.at<double>(1, 1);
	double r_max = std::sqrt(x0*x0 + y0*y0);

	for (double r_mm = 0.01; r_mm < 1.2; r_mm = r_mm + 0.01)
	{
		double r = r_mm / 1.2 * r_max;
		double kr = 1 + distCoeffs.at<double>(0, 0) * pow(r, 2) + distCoeffs.at<double>(1, 0) * pow(r, 4) + distCoeffs.at<double>(4, 0) * pow(r, 6);
		CV_Assert(kr > 0 && kr <= 1);
		double distortPercentage = (1 / kr - 1) * 100;
		fs_out_distort_data << setprecision(numeric_limits<double>::digits10 + 1) << std::scientific
			<< r << "  " << r_mm << "  " << distortPercentage << std::endl;
	}
	fs_out_distort_data.close();

	std::cout << "Re-projection error reported by calibrateCamera: " << rms << endl;

	bool ok = checkRange(cameraMatrix) && checkRange(distCoeffs);

	totalAvgErr = computeReprojectionErrors(objectPoints, imagePoints, rvecs, tvecs, cameraMatrix,
		distCoeffs, reprojErrs, s.useFisheye);

	return ok;
}

void Cowa_Calib::calcBoardCornerPositions(Size boardSize, float squareSize, vector<Point3f>& corners,
	Settings::Pattern patternType /*= Settings::CHESSBOARD*/)
{
	corners.clear();

	switch (patternType)
	{
	case Settings::CHESSBOARD:
	case Settings::CIRCLES_GRID:
		for (int i = 0; i < boardSize.height; ++i)
			for (int j = 0; j < boardSize.width; ++j)
				corners.push_back(Point3f(j*squareSize, i*squareSize, 0));
		break;

	case Settings::ASYMMETRIC_CIRCLES_GRID:
		for (int i = 0; i < boardSize.height; i++)
			for (int j = 0; j < boardSize.width; j++)
				corners.push_back(Point3f((2 * j + i % 2)*squareSize, i*squareSize, 0));
		break;
	default:
		break;
	}
}

//! [compute_errors]
double Cowa_Calib::computeReprojectionErrors(const vector<vector<Point3f> >& objectPoints,
	const vector<vector<Point2f> >& imagePoints,
	const vector<Mat>& rvecs, const vector<Mat>& tvecs,
	const Mat& cameraMatrix, const Mat& distCoeffs,
	vector<float>& perViewErrors, bool fisheye)
{
	vector<Point2f> imagePoints2;
	size_t totalPoints = 0;
	double totalErr = 0, err;
	perViewErrors.resize(objectPoints.size());

	for (size_t i = 0; i < objectPoints.size(); ++i)
	{
		if (fisheye)
		{
			fisheye::projectPoints(objectPoints[i], imagePoints2, rvecs[i], tvecs[i], cameraMatrix,
				distCoeffs);
		}
		else
		{
			projectPoints(objectPoints[i], rvecs[i], tvecs[i], cameraMatrix, distCoeffs, imagePoints2);
		}
		err = norm(imagePoints[i], imagePoints2, NORM_L2);

		size_t n = objectPoints[i].size();
		perViewErrors[i] = (float)std::sqrt(err*err / n);
		totalErr += err*err;
		totalPoints += n;
	}

	return std::sqrt(totalErr / totalPoints);
}

void Cowa_Calib::saveCameraParams(Settings& s, Size& imageSize, Mat& cameraMatrix, Mat& distCoeffs,
	const vector<Mat>& rvecs, const vector<Mat>& tvecs,
	const vector<float>& reprojErrs, const vector<vector<Point2f> >& imagePoints,
	double totalAvgErr)
{
	FileStorage fs(s.outputFileName, FileStorage::WRITE);

	time_t tm;
	time(&tm);
	struct tm *t2 = localtime(&tm);
	char buf[1024];
	strftime(buf, sizeof(buf), "%c", t2);

	fs << "calibration_time" << buf;

	if (!rvecs.empty() || !reprojErrs.empty())
		fs << "nr_of_frames" << (int)max(rvecs.size(), reprojErrs.size());
	fs << "image_width" << imageSize.width;
	fs << "image_height" << imageSize.height;
	fs << "board_width" << s.boardSize.width;
	fs << "board_height" << s.boardSize.height;
	fs << "square_size" << s.squareSize;

	if (s.flag & CALIB_FIX_ASPECT_RATIO)
		fs << "fix_aspect_ratio" << s.aspectRatio;

	if (s.flag)
	{
		if (s.useFisheye)
		{
			sprintf(buf, "flags:%s%s%s%s%s%s",
				s.flag & fisheye::CALIB_FIX_SKEW ? " +fix_skew" : "",
				s.flag & fisheye::CALIB_FIX_K1 ? " +fix_k1" : "",
				s.flag & fisheye::CALIB_FIX_K2 ? " +fix_k2" : "",
				s.flag & fisheye::CALIB_FIX_K3 ? " +fix_k3" : "",
				s.flag & fisheye::CALIB_FIX_K4 ? " +fix_k4" : "",
				s.flag & fisheye::CALIB_RECOMPUTE_EXTRINSIC ? " +recompute_extrinsic" : "");
		}
		else
		{
			sprintf(buf, "flags:%s%s%s%s",
				s.flag & CALIB_USE_INTRINSIC_GUESS ? " +use_intrinsic_guess" : "",
				s.flag & CALIB_FIX_ASPECT_RATIO ? " +fix_aspectRatio" : "",
				s.flag & CALIB_FIX_PRINCIPAL_POINT ? " +fix_principal_point" : "",
				s.flag & CALIB_ZERO_TANGENT_DIST ? " +zero_tangent_dist" : "");
		}
		cvWriteComment(*fs, buf, 0);
	}

	fs << "flags" << s.flag;

	fs << "fisheye_model" << s.useFisheye;

	fs << "camera_matrix" << cameraMatrix;
	fs << "distortion_coefficients" << distCoeffs;

	fs << "avg_reprojection_error" << totalAvgErr;
	if (s.writeExtrinsics && !reprojErrs.empty())
		fs << "per_view_reprojection_errors" << Mat(reprojErrs);

	if (s.writeExtrinsics && !rvecs.empty() && !tvecs.empty())
	{
		CV_Assert(rvecs[0].type() == tvecs[0].type());
		Mat bigmat((int)rvecs.size(), 6, rvecs[0].type());
		for (size_t i = 0; i < rvecs.size(); i++)
		{
			Mat r = bigmat(Range(int(i), int(i + 1)), Range(0, 3));
			Mat t = bigmat(Range(int(i), int(i + 1)), Range(3, 6));

			CV_Assert(rvecs[i].rows == 3 && rvecs[i].cols == 1);
			CV_Assert(tvecs[i].rows == 3 && tvecs[i].cols == 1);
			//*.t() is MatExpr (not Mat) so we can use assignment operator
			r = rvecs[i].t();
			t = tvecs[i].t();
		}
		//cvWriteComment( *fs, "a set of 6-tuples (rotation vector + translation vector) for each view", 0 );
		fs << "extrinsic_parameters" << bigmat;
	}

	if (s.writePoints && !imagePoints.empty())
	{
		Mat imagePtMat((int)imagePoints.size(), (int)imagePoints[0].size(), CV_32FC2);
		for (size_t i = 0; i < imagePoints.size(); i++)
		{
			Mat r = imagePtMat.row(int(i)).reshape(2, imagePtMat.cols);
			Mat imgpti(imagePoints[i]);
			imgpti.copyTo(r);
		}
		fs << "image_points" << imagePtMat;
	}
}

//! [run_and_save]
bool Cowa_Calib::runCalibrationAndSave(Settings& s, Size imageSize, Mat& cameraMatrix, Mat& distCoeffs,
	vector<vector<Point2f> > imagePoints)
{
	vector<Mat> rvecs, tvecs;
	vector<float> reprojErrs;
	double totalAvgErr = 0;

	bool ok = runCalibration(s, imageSize, cameraMatrix, distCoeffs, imagePoints, rvecs, tvecs, reprojErrs,
		totalAvgErr);
	cout << (ok ? "Calibration succeeded" : "Calibration failed")
		<< ". avg re projection error = " << totalAvgErr << endl;

	if (ok)
		saveCameraParams(s, imageSize, cameraMatrix, distCoeffs, rvecs, tvecs, reprojErrs, imagePoints,
		totalAvgErr);
	return ok;
}
//! [run_and_save]


/*----------------------------
* 功能 : 激光标定
*----------------------------
* 函数 : RunCalibrateLaser
* 访问 : public
* 返回 : 0
*
* 参数 : inputCameraDataFile		 [in]    相机内外参数
* 参数 : inputImageListFile		 [in]    配置文件
* 参数 : outputLaserCameraFile   [in]    相机到激光面的旋转平移矩阵
* 参数 : outputBinFile			 [in]    bin文件
*/
COWACALIB_API int  Cowa_Calib::RunCalibrateLaser(const string inputCameraDataFile, const string inputImageListFile, const string outputLaserCameraFile, const string outputBinFile)
{
	//! [file_read]
	FileStorage fs(inputCameraDataFile, FileStorage::READ); // Read the settings
	if (!fs.isOpened())
	{
		cout << "Could not open the configuration file: \"" << inputCameraDataFile << "\"" << endl;
		return -1;
	}
	fs["nr_of_frames"] >> nFrame;
	fs["board_width"] >> board_width;
	fs["board_height"] >> board_height;
	fs["square_size"] >> square_size;
	fs["camera_matrix"] >> cameraMatrix;
	fs["image_points"] >> distortImagesPoints;
	fs["distortion_coefficients"] >> distcofficients;
	fs["extrinsic_parameters"] >> extrinsicMatrixs;

	distortImagesPoints.copyTo(imagesPoints);

	double undistortImagePointsError = 0.0;
	for (int i = 0; i < distortImagesPoints.rows; i++)
	{
		for (int j = 0; j < distortImagesPoints.cols; j++)
		{
			cv::Point2f lDistortPoint = distortImagesPoints.at<cv::Point2f>(i, j);
			cv::Ptr<cv::DownhillSolver> solver = cv::DownhillSolver::create();
			cv::Ptr<cv::MinProblemSolver::Function> ptr_F = cv::makePtr<DistortF>(lDistortPoint.x, lDistortPoint.y);
			cv::Mat solution = (cv::Mat_<double>(1, 2) << lDistortPoint.x, lDistortPoint.y),
				step = (cv::Mat_<double>(2, 1) << -0.5, -0.5);


			solver->setFunction(ptr_F);
			solver->setInitStep(step);
			double res = solver->minimize(solution);

			CV_Assert(res < 0.005);
			imagesPoints.at<cv::Point2f>(i, j) = cv::Point2f(solution.at<double>(0, 0), solution.at<double>(0, 1));
			undistortImagePointsError += res;
		}
	}
	undistortImagePointsError = undistortImagePointsError / (distortImagesPoints.cols * distortImagesPoints.rows);
	std::cout << "Undistort ImagePoints Error is: " << undistortImagePointsError << std::endl;

	initUndistortRectifyMap(cameraMatrix, distcofficients, cv::Mat::eye(3, 3, CV_64FC1), cameraMatrix, cv::Size(640, 480), CV_32FC1, undistorMapX, undistorMapY);

	imgList = new cv::Mat[nFrame];
	imgListUndistort = new cv::Mat[nFrame];
	imgBinary = new cv::Mat[nFrame];

	laserPoints_CameraCoord3d = cv::Mat(board_width*nFrame, 3, CV_64FC1, Scalar(0));
	laserPoints2f = cv::Mat(board_width*nFrame, 2, CV_32FC1, Scalar(0));

	{
		FileStorage fsImageList(inputImageListFile, FileStorage::READ);
		if (!fsImageList.isOpened())
		{
			cout << "Could not open the configuration file: \"" << inputImageListFile << "\"" << endl;
			return -1;
		}
		FileNode node = fsImageList.getFirstTopLevelNode();
		if (node.type() != FileNode::SEQ)
			return -1;
		FileNodeIterator it = node.begin(), it_end = node.end();
		int idx = 0;
		for (; it != it_end, idx < nFrame; ++it, idx++)
			imgList[idx] = imread((string)*it);

		if (idx != nFrame)
		{
			cout << "frame number not match, camera frame is " << nFrame << ", laser frame is " << idx << endl;
			return -1;
		}
	}


	for (int i = 0; i < nFrame; i++)
	{
		cv::cvtColor(imgList[i], imgList[i], cv::COLOR_BGR2GRAY);
		remap(imgList[i], imgListUndistort[i], undistorMapX, undistorMapY, INTER_LINEAR, BORDER_CONSTANT);
	}

	for (int i = 0; i < nFrame; i++)
	{
		std::cout << "Processing img " << i << std::endl;
		int pos = 40;
		cv::imshow("Threshold Results", imgListUndistort[i]);
		cv::createTrackbar("threshold trackbar", "Threshold Results", &pos, 255, onThresholdChanged, &i);

		cv::setMouseCallback("Threshold Results", onMouseCallback, &i);

		cv::waitKey(-1);
	}

	cv::Mat b(nFrame*board_width, 1, CV_64FC1, Scalar(200));
	cv::Mat sol;

	cv::Mat A = laserPoints_CameraCoord3d.rowRange(0, validLaserPointCount);
	cv::solve(A, b, sol, DECOMP_SVD);
	double BPError = 0;
	for (int i = 0; i < A.rows; i++)
	{
		cv::Mat res = ((A.row(i) * sol));
		BPError += abs(res.at<double>(0) - 200) / cv::norm(sol);
	}
	BPError = BPError / (validLaserPointCount);

	std::cout << "Back Projection Error is: " << BPError << std::endl;
	std::cout << "laser plane (in camera coordinate) is : " << sol.at<double>(0) << ", " << sol.at<double>(1) << ", " << sol.at<double>(2) << "   " << 200 << std::endl;

	cv::Mat oriLaserPlane(4, 1, CV_64FC1);
	oriLaserPlane.at<double>(0) = sol.at<double>(0);
	oriLaserPlane.at<double>(1) = sol.at<double>(1);
	oriLaserPlane.at<double>(2) = sol.at<double>(2);
	oriLaserPlane.at<double>(3) = -200;

	if (oriLaserPlane.at<double>(1) > 0) // we should make sure that the normal of plane point to the negative y direction
	{
		oriLaserPlane = -oriLaserPlane;
	}

	getAndSaveTransformation(oriLaserPlane, outputLaserCameraFile, outputBinFile);

	delete[] imgList;
	delete[] imgListUndistort;
	delete[] imgBinary;


	return 0;
}

cv::Mat Cowa_Calib::getNearestPlanePoint(cv::Vec4d plane, cv::Vec3d point)
{
	cv::Mat A(3, 3, CV_64FC1);
	cv::Mat b(3, 1, CV_64FC1);
	A.at<double>(0, 0) = plane[0];
	A.at<double>(0, 1) = plane[1];
	A.at<double>(0, 2) = plane[2];
	b.at<double>(0) = -plane[3];

	A.at<double>(1, 0) = plane[1];
	A.at<double>(1, 1) = -plane[0];
	A.at<double>(1, 2) = 0;
	b.at<double>(1) = plane[1] * point[0] - plane[0] * point[1];

	A.at<double>(2, 0) = 0;
	A.at<double>(2, 1) = plane[2];
	A.at<double>(2, 2) = -plane[1];
	b.at<double>(2) = plane[2] * point[1] - plane[1] * point[2];

	cv::Mat sol;
	cv::solve(A, b, sol);

	return sol;
}

cv::Mat Cowa_Calib::getRotationMatix(cv::Mat originVec, cv::Mat expectedVec)
{
	cv::Mat rotationAxis = originVec.cross(expectedVec);
	rotationAxis = rotationAxis / cv::norm(rotationAxis);

	double rotationAngle = std::acos(originVec.dot(expectedVec) / cv::norm(originVec) / cv::norm(expectedVec));

	cv::Mat omega(3, 3, CV_64FC1, Scalar(0));
	omega.at<double>(0, 1) = -rotationAxis.at<double>(2);
	omega.at<double>(0, 2) = rotationAxis.at<double>(1);
	omega.at<double>(1, 0) = rotationAxis.at<double>(2);
	omega.at<double>(1, 2) = -rotationAxis.at<double>(0);
	omega.at<double>(2, 0) = -rotationAxis.at<double>(1);
	omega.at<double>(2, 1) = rotationAxis.at<double>(0);
	cv::Mat rotationMatrix = cv::Mat::eye(3, 3, CV_64FC1) + std::sin(rotationAngle)*omega + (1 - std::cos(rotationAngle))*omega*omega;

	cv::Mat normedOri = originVec / cv::norm(originVec);
	cv::Mat normedExp = expectedVec / cv::norm(expectedVec);

	double BP = cv::norm(rotationMatrix*normedOri - normedExp);

	std::cout << "BP error of getTransformationMatrix of two vector is: " << BP << std::endl;
	return rotationMatrix;
}

Mat Cowa_Calib::comMatC(Mat Matrix1, Mat Matrix2, Mat &MatrixCom)
{
	CV_Assert(Matrix1.cols == Matrix2.cols);//列数不相等，出现错误中断    
	MatrixCom.create(Matrix1.rows + Matrix2.rows, Matrix1.cols, Matrix1.type());
	Mat temp = MatrixCom.rowRange(0, Matrix1.rows);
	Matrix1.copyTo(temp);
	Mat temp1 = MatrixCom.rowRange(Matrix1.rows, Matrix1.rows + Matrix2.rows);
	Matrix2.copyTo(temp1);
	return MatrixCom;
}

bool Cowa_Calib::isLittleEndian()
{
	short a = 0x1122; //十六进制，一个数值占4位
	char b = *(char *)&a; //通过将short(2字节)强制类型转换成char单字节，b指向a的起始字节（低字节）
	if (b == 0x11) //低字节存的是数据的高字节数据
	{
		return false;
	}
	else
	{
		return true;
	}
}

Mat Cowa_Calib::comMatR(Mat Matrix1, Mat Matrix2, Mat &MatrixCom)
{

	CV_Assert(Matrix1.rows == Matrix2.rows);//行数不相等，出现错误中断    
	MatrixCom.create(Matrix1.rows, Matrix1.cols + Matrix2.cols, Matrix1.type());
	Mat temp = MatrixCom.colRange(0, Matrix1.cols);
	Matrix1.copyTo(temp);
	Mat temp1 = MatrixCom.colRange(Matrix1.cols, Matrix1.cols + Matrix2.cols);
	Matrix2.copyTo(temp1);
	return MatrixCom;
}

void Cowa_Calib::QUADBYTESSWAP(char src[4], char dst[4])
{
	dst[0] = src[3];
	dst[1] = src[2];
	dst[2] = src[1];
	dst[3] = src[0];
}

#define UNDISTORT_FAILED_THRES 0.1
void Cowa_Calib::getAndSaveTransformation(cv::Mat LaserPlane, std::string outputLaserCameraFile, std::string outputBinFile)
{

	cv::Mat T = getNearestPlanePoint(cv::Vec4d(LaserPlane), cv::Vec3d(0, 0, 0));
	cv::Mat R = getRotationMatix(cv::Mat(cv::Vec3d(0, 0, 1)), LaserPlane.rowRange(0, 3));

	FileStorage fsSave;

	fsSave.open(outputLaserCameraFile, FileStorage::WRITE);
	if (fsSave.isOpened())
	{
		cv::Mat H_lc;
		cv::Mat C = (Mat_<double>(1, 4) << 0, 0, 0, 1);
		comMatR(R, T, H_lc);
		comMatC(H_lc, C, H_lc);
		fsSave << "LaserCamera" << H_lc;
		fsSave.release();
	}
	else
	{
		cout << "Error: can not save the extrinsic parameters\n";
		CV_Assert(false);
	}
	/*cv::Mat expectedLaserPlane(4, 1, CV_64FC1);
	expectedLaserPlane.at<double>(0) = 0;
	expectedLaserPlane.at<double>(1) = 0;
	expectedLaserPlane.at<double>(2) = 1;
	expectedLaserPlane.at<double>(3) = 0;

	cv::Mat translate = cv::Mat(3, 1, CV_64FC1, Scalar(0));
	translate.at<double>(1) = -(-oriLaserPlane.at<double>(3) / oriLaserPlane.at<double>(1));
	cv::Mat RotationMatrix = getRotationMatix(oriLaserPlane.rowRange(0,3), expectedLaserPlane.rowRange(0,3));

	cv::Mat R = RotationMatrix.inv();
	cv::Mat T = -1 * translate; */


	// see visual sensing P59
	double fx = cameraMatrix.at<double>(0, 0);
	double fy = cameraMatrix.at<double>(1, 1);
	double cx = cameraMatrix.at<double>(0, 2);
	double cy = cameraMatrix.at<double>(1, 2);
	double r1 = R.at<double>(0, 0);
	double r2 = R.at<double>(0, 1);
	double r3 = R.at<double>(0, 2);
	double r4 = R.at<double>(1, 0);
	double r5 = R.at<double>(1, 1);
	double r6 = R.at<double>(1, 2);
	double r7 = R.at<double>(2, 0);
	double r8 = R.at<double>(2, 1);
	double r9 = R.at<double>(2, 2);
	double tx = T.at<double>(0);
	double ty = T.at<double>(1);
	double tz = T.at<double>(2);

	double p1 = fx * r1;
	double p2 = fx * r2;
	double p3 = fx * tx;
	double p4 = fy * r4;
	double p5 = fy * r5;
	double p6 = fy * ty;

	std::cout << "Laser Point in Camera Coordinate and in Laser Coordinate" << std::endl;
	double BPError = 0;
	for (int i = 0; i < validLaserPointCount; i++)
	{
		double u = laserPoints2f.at<float>(i, 0);
		double v = laserPoints2f.at<float>(i, 1);

		double u_ = u - cx;
		double v_ = v - cy;

		double m1 = u_*r7 - p1;
		double m2 = p2 - u_*r8;
		double m3 = p3 - u_*tz;
		double m4 = v_*r7 - p4;
		double m5 = p5 - v_*r8;
		double m6 = p6 - v_*tz;

		double Y_w = (m1*m6 - m3*m4) / (m2*m4 - m1*m5);
		double X_w = (m2*Y_w + m3) / m1;

		// BP test:
		cv::Point3d lPoint_CamCoord(laserPoints_CameraCoord3d.at<double>(i, 0), laserPoints_CameraCoord3d.at<double>(i, 1), laserPoints_CameraCoord3d.at<double>(i, 2));

		cv::Mat lPoint_LaserCoordMat(3, 1, CV_64FC1, Scalar(0));
		lPoint_LaserCoordMat.at<double>(0) = X_w;
		lPoint_LaserCoordMat.at<double>(1) = Y_w;
		cv::Mat lPoint_BPCamCoordMat = R * lPoint_LaserCoordMat + T;
		cv::Point3d lPoint_BPCamCoord(lPoint_BPCamCoordMat.at<double>(0), lPoint_BPCamCoordMat.at<double>(1), lPoint_BPCamCoordMat.at<double>(2));

		std::cout << "CAM3d: " << lPoint_CamCoord.x << ", " << lPoint_CamCoord.y << ", " << lPoint_CamCoord.z <<
			"  LASER3D: " << X_w << ", " << Y_w << ", " << 0 << std::endl <<
			"BPC3D: " << lPoint_BPCamCoord.x << ", " << lPoint_BPCamCoord.y << ", " << lPoint_BPCamCoord.z << std::endl;

		BPError += cv::norm(lPoint_BPCamCoord - lPoint_CamCoord);
	}
	BPError = BPError / validLaserPointCount;
	std::cout << "BP error of Laser Coord to Camera Coord is: " << BPError << std::endl;

	// obtain transformation map
	cv::Mat transformMap(imgList[0].rows + 1, imgList[0].cols + 1, CV_32FC2);
	int lFaliedUndistCount = 0;
	cv::Mat undistorSuccessMask(transformMap.size(), CV_8UC1, cv::Scalar(255));
	cv::Mat undistorCoordAndRes(transformMap.size(), CV_32FC3, cv::Scalar(0.0, 0.0, 0.0));
	for (int y = 0; y < imgList[0].rows; y++)
	{
		for (int x = 0; x < imgList[0].cols; x++)
		{
			cv::Ptr<cv::DownhillSolver> solver = cv::DownhillSolver::create();
			cv::Ptr<cv::MinProblemSolver::Function> ptr_F = cv::makePtr<DistortF>(x, y);
			cv::Mat solution = (cv::Mat_<double>(1, 2) << x, y),
				step = (cv::Mat_<double>(2, 1) << -0.5, -0.5);

			solver->setFunction(ptr_F);
			solver->setInitStep(step);
			double res = solver->minimize(solution);
			if (std::fabs(res) > UNDISTORT_FAILED_THRES)
			{
				lFaliedUndistCount++;
				undistorCoordAndRes.at<cv::Vec3f>(y, x) = cv::Vec3f(solution.at<double>(0, 0), solution.at<double>(0, 1), res);
				transformMap.at<cv::Vec2f>(y, x) = cv::Point2f(-FLT_MAX, -FLT_MAX);
				undistorSuccessMask.at<uchar>(y, x) = 0;
				continue;
			}
			undistorCoordAndRes.at<cv::Vec3f>(y, x) = cv::Vec3f(solution.at<double>(0, 0), solution.at<double>(0, 1), res);
			// CV_Assert(res < 0.001);

			// solution is undistort u & v

			double u = solution.at<double>(0, 0);
			double v = solution.at<double>(0, 1);

			double u_ = u - cx;
			double v_ = v - cy;

			double m1 = u_*r7 - p1;
			double m2 = p2 - u_*r8;
			double m3 = p3 - u_*tz;
			double m4 = v_*r7 - p4;
			double m5 = p5 - v_*r8;
			double m6 = p6 - v_*tz;

			double Y_w = (m1*m6 - m3*m4) / (m2*m4 - m1*m5);
			double X_w = (m2*Y_w + m3) / m1;

			transformMap.at<cv::Vec2f>(y, x) = cv::Point2f(X_w, Y_w);
		}
		transformMap.at<cv::Point2f>(y, transformMap.cols - 1) = transformMap.at<cv::Point2f>(y, transformMap.cols - 2);
	}
	for (int x = 0; x < transformMap.cols; x++)
	{
		transformMap.at<cv::Point2f>(transformMap.rows - 1, x) = transformMap.at<cv::Point2f>(transformMap.rows - 2, x);
	}

	cv::Mat savedTransformationMap(transformMap.rows, transformMap.cols, CV_32FC3);
	for (int y = 0; y < transformMap.rows; y++)
	{
		for (int x = 0; x < transformMap.cols; x++)
		{
			savedTransformationMap.at<cv::Vec3f>(y, x) = cv::Vec3f(transformMap.at<Vec2f>(y, x)[0], transformMap.at<Vec2f>(y, x)[1], 0);
		}
	}


	// save transformaton map; // in little-endian mode
	FILE* fp = fopen(outputBinFile.c_str(), "wb");
	float* pDataTransformationMap = (float*)savedTransformationMap.data;
	if (isLittleEndian())
	{
		fwrite((void*)pDataTransformationMap, 4, transformMap.size().area() * 3, fp);
	}
	else // host is big Endian, transform it to little Endian when write to bin file
	{
		char tmpbuf[4];
		for (int i = 0; i < transformMap.size().area() * 3; i++, pDataTransformationMap++)
		{
			QUADBYTESSWAP((char*)pDataTransformationMap, tmpbuf);
			fwrite(tmpbuf, 1, 4, fp);
		}
	}
	fclose(fp);
	return;
}

void onThresholdChanged(int pos, void* data)
{
	int index = *((int *)data);
	cv::threshold(imgListUndistort[index], imgBinary[index], pos, 255, cv::THRESH_BINARY);
	imshow("Threshold Results", imgBinary[index]);
}

cv::Vec2f findLines(cv::Mat& img, cv::Point startPoint, cv::Point endPoint)
{
	// 线使用极坐标表示方式
	double lTheta;
	if (endPoint.x == startPoint.x)
		lTheta = CV_PI / 2;
	else
		lTheta = std::atan(((double)endPoint.y - startPoint.y) / (endPoint.x - startPoint.x));

	lTheta += CV_PI / 2;
	double lRho = startPoint.x * cos(lTheta) + startPoint.y * sin(lTheta);

	cv::Mat results(img.rows, img.cols, CV_8UC3, cv::Scalar(0));

	std::vector<cv::Point> lPoints;
	uchar* pImg = img.data;
	for (int y = 0; y < img.rows; y++)
	{
		for (int x = 0; x < img.cols; x++, pImg++)
		{
			if (*pImg == 255)
			{
				if (std::abs(lRho - (x*cos(lTheta) + y*sin(lTheta))) < 5)
				{
					lPoints.push_back(cv::Point(x, y));
					results.at<cv::Vec3b>(y, x) = cv::Vec3b(255, 255, 255);
				}
			}
		}
	}

	cv::Vec4f line;
	cv::fitLine(lPoints, line, cv::DIST_L2, 0, 0.01, 0.01);

	cv::line(results, cv::Point(line[2] - 1000 * line[0], line[3] - 1000 * line[1]), cv::Point(line[2] + 1000 * line[0], line[3] + 1000 * line[1]), cv::Scalar(0, 0, 255));
	cv::imshow("Find Line Results", results);
	cv::waitKey(10);

	double lResultTheta;
	if (abs(line[0]) < 0.00001)
		lResultTheta = CV_PI / 2;
	else
		lResultTheta = std::atan(line[1] / line[0]);

	lResultTheta += CV_PI / 2;
	double lResultRho = line[2] * cos(lTheta) + line[3] * sin(lTheta);

	return cv::Vec2f(lResultRho, lResultTheta);
}

/**
* Help Function to get intersect point of two lines
*
* @param  line1:  input first line
* @param  line2:  input second line
* @return intersect point
*/
cv::Point2d hGetIntersectPoint(cv::Vec2f line1, cv::Vec2f line2)
{
	cv::Point2d ret;

	double A_data[] = { cos(line1[1]), sin(line1[1]), cos(line2[1]), sin(line2[1]) };
	double b_data[] = { line1[0], line2[0] };
	cv::Mat A(2, 2, CV_64FC1, A_data);
	cv::Mat b(2, 1, CV_64FC1, b_data);

	cv::Mat sol;
	cv::solve(A, b, sol);

	ret.x = sol.at<double>(0, 0);
	ret.y = sol.at<double>(1, 0);

	return ret;
}

cv::Point2d hGetIntersectPoint(cv::Vec2f line1, cv::Point2f line2_start, cv::Point2f line2_end)
{
	float lTheta;
	if (abs(line2_start.x - line2_end.x) < 0.00001)
		lTheta = CV_PI / 2;
	else
		lTheta = atan((line2_end.y - line2_start.y) / (line2_end.x - line2_start.x));

	lTheta += CV_PI / 2;

	float lRho = line2_start.x * cos(lTheta) + line2_start.y * sin(lTheta);
	return hGetIntersectPoint(line1, Vec2f(lRho, lTheta));
}

void onMouseCallback(int event, int x, int y, int flags, void* userdata)
{
	static cv::Point lStartPoint;
	int index = *((int *)userdata);
	switch (event)
	{
	case cv::EVENT_LBUTTONDOWN:
	{
		lStartPoint = cv::Point(x, y);
		cv::circle(imgBinary[index], cv::Point(x, y), 5, cv::Scalar(200));
		imshow("Threshold Results", imgBinary[index]);
		break;
	}
	case cv::EVENT_RBUTTONDOWN:
	{
		cv::circle(imgBinary[index], cv::Point(x, y), 5, cv::Scalar(100));
		imshow("Threshold Results", imgBinary[index]);
		Vec2f lLaserLine = findLines(imgBinary[index], lStartPoint, cv::Point(x, y));
		float lLaserLineRho = lLaserLine[0];
		float lLaserLineTheta = lLaserLine[1];

		cv::Mat findPointsResults(imgBinary[index].rows, imgBinary[index].cols, CV_8UC3, Scalar(0));
		cv::line(findPointsResults, cv::Point2f(0, lLaserLineRho / sin(lLaserLineTheta)),
			cv::Point2f(10000, (lLaserLineRho - 10000 * cos(lLaserLineTheta)) / sin(lLaserLineTheta)), Scalar(0, 0, 255));


		cv::Mat lPoints = imagesPoints.row(index);
		// 遍历棋盘格内角点，找到激光线与棋盘格竖直线的交点
		for (int x = 0; x < board_width; x++)
		{
			for (int y = 0; y < board_height - 1; y++)
			{
				// x相邻两个角点
				cv::Point2f lPointUp = cv::Point2f(lPoints.at<Vec2f>(x + y * board_width));
				cv::Point2f lPointDown = cv::Point2f(lPoints.at<Vec2f>(x + (y + 1) * board_width));
				cv::circle(findPointsResults, lPointUp, 3, Scalar(0, 255, 0));
				cv::circle(findPointsResults, lPointDown, 3, Scalar(0, 255, 0));

				float lRho1 = lPointUp.x * cos(lLaserLineTheta) + lPointUp.y * sin(lLaserLineTheta);
				float lRho2 = lPointDown.x * cos(lLaserLineTheta) + lPointDown.y * sin(lLaserLineTheta);

				if ((lRho1 - lLaserLineRho) * (lRho2 - lLaserLineRho) < 0) //交点在相邻两个角点中间。
				{
					cv::Point2f lPointIntersected;
					cv::Point3d lPointIntersected_w;

					lPointIntersected = hGetIntersectPoint(lLaserLine, lPointUp, lPointDown);
					laserPoints2f.at<float>(validLaserPointCount, 0) = lPointIntersected.x;
					laserPoints2f.at<float>(validLaserPointCount, 1) = lPointIntersected.y;
					cv::circle(findPointsResults, lPointIntersected, 3, Scalar(0, 0, 255), -1);

					cv::Point2f lPoint0, lPoint1, lPoint2, lPoint3;
					cv::Point3f lPoint0_w, lPoint1_w, lPoint2_w, lPoint3_w;
					if (y == 0)
					{
						lPoint0 = cv::Point2f(lPoints.at<Vec2f>(x + y * board_width));
						lPoint2 = cv::Point2f(lPoints.at<Vec2f>(x + (y + 1) * board_width));
						lPoint3 = cv::Point2f(lPoints.at<Vec2f>(x + (y + 2)* board_width));

						lPoint0_w = cv::Point3f(x*square_size, y*square_size, 0);
						lPoint2_w = cv::Point3f(x*square_size, (y + 1)*square_size, 0);
						lPoint3_w = cv::Point3f(x*square_size, (y + 2)*square_size, 0);

						cv::circle(findPointsResults, lPoint0, 5, Scalar(0, 0, 255));
						cv::circle(findPointsResults, lPoint2, 5, Scalar(0, 0, 255));
						cv::circle(findPointsResults, lPoint3, 5, Scalar(0, 0, 255));

						lPoint1 = lPointIntersected;
						// 根据 cross ratio 来计算lPointIntersected_w, 参见《visual sensing and its application》 第64页 
						double numerator = cv::norm(lPoint0 - lPoint2) / cv::norm(lPoint1 - lPoint2);
						double denominator = cv::norm(lPoint0 - lPoint3) / cv::norm(lPoint1 - lPoint3);
						double ratio = numerator / denominator;
						double t0 = 0;
						double t2 = 1;
						double t3 = 2;

						double t1 = (ratio*(t0 - t3)*t2 - t3*(t0 - t2)) /
							(ratio*(t0 - t3) - (t0 - t2));
						lPointIntersected_w = lPoint0_w + t1*(lPoint2_w - lPoint0_w);
						lPoint1_w = lPointIntersected_w;

					}
					else
					{
						lPoint0 = cv::Point2f(lPoints.at<Vec2f>(x + (y - 1)* board_width));
						lPoint1 = cv::Point2f(lPoints.at<Vec2f>(x + y * board_width));
						lPoint3 = cv::Point2f(lPoints.at<Vec2f>(x + (y + 1)* board_width));

						lPoint0_w = cv::Point3f(x*square_size, (y - 1)*square_size, 0);
						lPoint1_w = cv::Point3f(x*square_size, y*square_size, 0);
						lPoint3_w = cv::Point3f(x*square_size, (y + 1)*square_size, 0);

						cv::circle(findPointsResults, lPoint0, 5, Scalar(0, 0, 255));
						cv::circle(findPointsResults, lPoint1, 5, Scalar(0, 0, 255));
						cv::circle(findPointsResults, lPoint3, 5, Scalar(0, 0, 255));

						lPoint2 = lPointIntersected;
						// 根据 cross ratio 来计算lPointIntersected_w, 参见《visual sensing and its application》 第64页 
						double numerator = cv::norm(lPoint0 - lPoint2) / cv::norm(lPoint1 - lPoint2);
						double denominator = cv::norm(lPoint0 - lPoint3) / cv::norm(lPoint1 - lPoint3);
						double ratio = numerator / denominator;
						double t0 = 0;
						double t1 = 1;
						double t3 = 2;

						double t2 = (ratio*(t0 - t3)*t1 - t0*(t1 - t3)) /
							(ratio*(t0 - t3) - (t1 - t3));
						lPointIntersected_w = lPoint0_w + t2*(lPoint1_w - lPoint0_w);
						lPoint2_w = lPointIntersected_w;
					}

					stringstream ss;
					ss.setf(std::ios::fixed);
					ss.precision(1);
					ss << lPointIntersected_w.x << " " << lPointIntersected_w.y;
					cv::putText(findPointsResults, ss.str(), lPointIntersected, cv::FONT_HERSHEY_PLAIN, 0.8, cv::Scalar(0, 0, 255));

					cv::Mat rotationMatrix;
					cv::Rodrigues(extrinsicMatrixs.row(index).colRange(0, 3), rotationMatrix);
					cv::Mat translationVec = extrinsicMatrixs.row(index).colRange(3, 6);

					cv::Mat lPointMat_ChessBoardCoord(lPointIntersected_w);
					cv::Mat lPointMat_CameraCoord = rotationMatrix * lPointMat_ChessBoardCoord + translationVec.t();

					cv::Point3d lPoint_CameraCoord;
					lPoint_CameraCoord.x = lPointMat_CameraCoord.at<double>(0);
					lPoint_CameraCoord.y = lPointMat_CameraCoord.at<double>(1);
					lPoint_CameraCoord.z = lPointMat_CameraCoord.at<double>(2);

					std::cout << x << ": " << lPoint_CameraCoord.x << ", " << lPoint_CameraCoord.y << ", " << lPoint_CameraCoord.z << std::endl;

					laserPoints_CameraCoord3d.at<double>(validLaserPointCount, 0) = lPoint_CameraCoord.x;
					laserPoints_CameraCoord3d.at<double>(validLaserPointCount, 1) = lPoint_CameraCoord.y;
					laserPoints_CameraCoord3d.at<double>(validLaserPointCount, 2) = lPoint_CameraCoord.z;

					validLaserPointCount++;
				}
			}
		}


		cv::imshow("Find Points Results", findPointsResults);
		cv::waitKey(10);
		break;
	}
	default:
		break;
	}


}

/*计算标定板上模块的实际物理坐标*/
void Cowa_Calib::calRealPoint(vector<vector<Point3f>>& obj, Settings& s)
{
	//  Mat imgpoint(boardheight, boardwidth, CV_32FC3,Scalar(0,0,0));  
	vector<Point3f> imgpoint;
	for (int rowIndex = 0; rowIndex < s.boardSize.height; rowIndex++)
	{
		for (int colIndex = 0; colIndex < s.boardSize.width; colIndex++)
		{
			//  imgpoint.at<Vec3f>(rowIndex, colIndex) = Vec3f(rowIndex * squaresize, colIndex*squaresize, 0);  
			imgpoint.push_back(Point3f(rowIndex * s.squareSize, colIndex * s.squareSize, 0));
		}
	}
	for (int imgIndex = 0; imgIndex < s.nrFrames; imgIndex++)
	{
		obj.push_back(imgpoint);
	}
}

void Cowa_Calib::outputCameraParam(Settings& s)
{
	FileStorage fsSave;

	fsSave.open(s.outputFileName, FileStorage::WRITE);
	if (fsSave.isOpened())
	{
		fsSave << "cameraMatrixL" << cameraMatrixL << "cameraDistcoeffL" << distCoeffL << "cameraMatrixR" << cameraMatrixR << "cameraDistcoeffR" << distCoeffR;
		fsSave << "R" << R << "T" << T << "Rl" << Rl << "Rr" << Rr << "Pl" << Pl << "Pr" << Pr << "Q" << Q;
		cout << "R=" << R << endl << "T=" << T << endl << "Rl=" << Rl << endl << "Rr=" << Rr << endl << "Pl=" << Pl << endl << "Pr=" << Pr << endl << "Q=" << Q << endl;
		fsSave.release();
	}
	else
		cout << "Error: can not save the extrinsic parameters\n";
}

void Cowa_Calib::sortConnerPoints(std::vector<cv::Point2f>& corners)
{
	if (corners[0].y > corners[corners.size() - 1].y)
	{
		for (int i = 0; i < corners.size() / 2; i++)
		{
			cv::Point2f tmp = corners[i];
			corners[i] = corners[corners.size() - 1 - i];
			corners[corners.size() - 1 - i] = tmp;
		}
	}
}
/*----------------------------
* 功能 : 双目标定
*----------------------------
* 函数 : RunStereoCalib
* 访问 : public
* 返回 : 0
*
* 参数 : inputCameraDataFile		 [in]    相机内外参数
* 参数 : inputSettingsFile		 [in]    配置文件
* 参数 : outStereoDataFilePath   [in]    相机间的旋转平移矩阵
*/
COWACALIB_API int  Cowa_Calib::RunStereoCalib(const string inputCameraDataFile, const string inputSettingsFile, const string outStereoDataFilePath){
	//! [file_read]
	FileStorage fs(inputCameraDataFile, FileStorage::READ); // Read the settings
	if (!fs.isOpened())
	{
		cout << "Could not open the configuration file: \"" << inputCameraDataFile << "\"" << endl;
		return -1;
	}
	
	fs["camera_matrix"] >> cameraMatrixL;
	fs["distortion_coefficients"] >> distCoeffL;

	cameraMatrixR = cameraMatrixL;
	distCoeffR = distCoeffL;

	//![file_read]
	Settings s;
	FileStorage fsSetting(inputSettingsFile, FileStorage::READ);     //Read the settings
	if (!fsSetting.isOpened())
	{
		cout << "Could not open the configuration file: \"" << inputSettingsFile << "\"" << endl;
		return -1;
	}
	fsSetting["Settings"] >> s;
	fsSetting.release();                               // close Settings file

	if (outStereoDataFilePath.size() > 1)
		s.outputFileName = outStereoDataFilePath;
	//[file_read]

	if (!s.goodInput)
	{
		cout << "Invalid input detected. Application stopping. " << endl;
		return -1;
	}

	int goodFrameCount = 0;
	Size imageSize;
	while (goodFrameCount < s.nrFrames)
	{
		char filename[100];
		//left img
		sprintf_s(filename, s.imageList[0].c_str());
		rgbImageL = imread(filename, CV_LOAD_IMAGE_COLOR);
		if (rgbImageL.empty())
			break;
		if (imageSize == Size())
			imageSize = rgbImageL.size();
		else if (rgbImageL.size() != imageSize)
		{
			cout << "The image " << filename << " has the size different from the first image size. Skipping the pair\n";
			break;
		}

		cvtColor(rgbImageL, grayImageL, CV_BGR2GRAY);

		//right img
		sprintf_s(filename, s.imageList[1].c_str());
		rgbImageR = imread(filename, CV_LOAD_IMAGE_COLOR);
		cvtColor(rgbImageR, grayImageR, CV_BGR2GRAY);

		bool isFindL, isFindR;

		isFindL = findChessboardCorners(rgbImageL, s.boardSize, cornerL);
		isFindR = findChessboardCorners(rgbImageR, s.boardSize, cornerR);
		
		sortConnerPoints(cornerL);
		sortConnerPoints(cornerR);

		if (isFindL == true && isFindR == true)  //如果两幅图像都找到了所有的角点 则说明这两幅图像是可行的  
		{
			/*
			Size(5,5) 搜索窗口的一半大小
			Size(-1,-1) 死区的一半尺寸
			TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 20, 0.1)		迭代终止条件
			*/
			cornerSubPix(grayImageL, cornerL, Size(5, 5), Size(-1, -1), TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 20, 0.1));
			drawChessboardCorners(rgbImageL, s.boardSize, cornerL, isFindL);
			imshow("chessboardL", rgbImageL);
			imagePointL.push_back(cornerL);


			cornerSubPix(grayImageR, cornerR, Size(5, 5), Size(-1, -1), TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 20, 0.1));
			drawChessboardCorners(rgbImageR, s.boardSize, cornerR, isFindR);
			imshow("chessboardR", rgbImageR);
			imagePointR.push_back(cornerR);

			goodFrameCount++;
			cout << "The image is good" << endl;
		}
		else
		{
			//LOG(WARNING) << "The stereo image you took does not come up to the requirements.";
			cout << "The image is bad please try again" << endl;
		}

		if (waitKey(10) == 'q')
		{
			break;
		}
	}

	//计算实际的校正点的三维坐标,根据实际标定格子的大小来设置
	calRealPoint(objRealPoint, s);
	cout << "cal real successful" << endl;

	//标定摄像头由于左右摄像机分别都经过了单目标定,所以在此处选择flag = CALIB_USE_INTRINSIC_GUESS
	double rms = stereoCalibrate(objRealPoint, imagePointL, imagePointR,
		cameraMatrixL, distCoeffL,
		cameraMatrixR, distCoeffR,
		Size(imageSize.width, imageSize.height), R, T, E, F,
		CALIB_USE_INTRINSIC_GUESS,
		TermCriteria(TermCriteria::COUNT + TermCriteria::EPS, 100, 1e-5));

	cout << "Stereo Calibration done with RMS error = " << rms << endl;

	stereoRectify(cameraMatrixL, distCoeffL, cameraMatrixR, distCoeffR, imageSize, R, T, Rl, Rr, Pl, Pr, Q,
		CALIB_ZERO_DISPARITY, -1, imageSize, &validROIL, &validROIR);
	
	initUndistortRectifyMap(cameraMatrixL, distCoeffL, Rl, Pr, imageSize, CV_32FC1, mapLx, mapLy);
	initUndistortRectifyMap(cameraMatrixR, distCoeffR, Rr, Pr, imageSize, CV_32FC1, mapRx, mapRy);

	Mat rectifyImageL, rectifyImageR;
	cvtColor(grayImageL, rectifyImageL, CV_GRAY2BGR);
	cvtColor(grayImageR, rectifyImageR, CV_GRAY2BGR);

	//imshow("Rectify Before", rectifyImageL);

	remap(rectifyImageL, rectifyImageL, mapLx, mapLy, INTER_LINEAR);
	remap(rectifyImageR, rectifyImageR, mapRx, mapRy, INTER_LINEAR);

	imshow("ImageL", rectifyImageL);
	imshow("ImageR", rectifyImageR);

	outputCameraParam(s);

	Mat canvas;
	double sf;
	int w, h;
	sf = 600. / MAX(imageSize.width, imageSize.height);
	w = cvRound(imageSize.width * sf);
	h = cvRound(imageSize.height * sf);
	canvas.create(h, w * 2, CV_8UC3);

	Mat canvasPart = canvas(Rect(w * 0, 0, w, h));                               
	resize(rectifyImageL, canvasPart, canvasPart.size(), 0, 0, INTER_AREA);      
	Rect vroiL(cvRound(validROIL.x*sf), cvRound(validROIL.y*sf),                      
		cvRound(validROIL.width*sf), cvRound(validROIL.height*sf));
	rectangle(canvasPart, vroiL, Scalar(0, 0, 255), 3, 8);                      

	canvasPart = canvas(Rect(w, 0, w, h));                                        
	resize(rectifyImageR, canvasPart, canvasPart.size(), 0, 0, INTER_LINEAR);
	Rect vroiR(cvRound(validROIR.x * sf), cvRound(validROIR.y*sf),
		cvRound(validROIR.width * sf), cvRound(validROIR.height * sf));
	rectangle(canvasPart, vroiR, Scalar(0, 255, 0), 3, 8);

	for (int i = 0; i < canvas.rows; i += 16)
		line(canvas, Point(0, i), Point(canvas.cols, i), Scalar(0, 255, 0), 1, 8);

	//imshow("rectified", canvas);

	cv::waitKey(10);
	return 0;
}


Mat Cowa_Calib::RotationMatrix(Mat& axis,
	const double& angleRad)
{
	//compute dimension of matrix
	double length = axis.cols;

	//check dimension of matrix
	if (length < 0)
	{
		//rotation matrix could not be computed because axis vector is not defined
		cout << "SOMETHING WENT WRONG!";
	}

	//normal axis vector 
	double f = 1.0 / length;
	double x = f * axis.at<double>(0);
	double y = f * axis.at<double>(1);
	double z = f * axis.at<double>(2);

	//compute rotation matrix
	double c = ::cos(angleRad);
	double s = ::sin(angleRad);
	double v = 1 - c;

	//3 * 3 null matrix
	Mat m = Mat::zeros(3, 3, CV_64FC1);

	m.at<double>(0, 0) = x*x*v + c;     m.at<double>(0, 1) = x*y*v - z*s;  m.at<double>(0, 2) = x*z*v + y*s;
	m.at<double>(1, 0) = x*y*v + z*s;   m.at<double>(1, 1) = y*y*v + c;    m.at<double>(1, 2) = y*z*v - x*s;
	m.at<double>(2, 0) = x*z*v - y*s;   m.at<double>(2, 1) = y*z*v + x*s;  m.at<double>(2, 2) = z*z*v + c;
	return m;

}

Mat Cowa_Calib::skew(Mat A)
{
	CV_Assert(A.cols == 1 && A.rows == 3);
	Mat B(3, 3, CV_64FC1);

	B.at<double>(0, 0) = 0.0;
	B.at<double>(0, 1) = -A.at<double>(2, 0);
	B.at<double>(0, 2) = A.at<double>(1, 0);

	B.at<double>(1, 0) = A.at<double>(2, 0);
	B.at<double>(1, 1) = 0.0;
	B.at<double>(1, 2) = -A.at<double>(0, 0);

	B.at<double>(2, 0) = -A.at<double>(1, 0);
	B.at<double>(2, 1) = A.at<double>(0, 0);
	B.at<double>(2, 2) = 0.0;

	return B;
}

void Cowa_Calib::Tsai_HandEye(Mat Hcg, vector<Mat> Hgij, vector<Mat> Hcij)
{
	CV_Assert(Hgij.size() == Hcij.size());
	int nStatus = Hgij.size();

	Mat Rgij(3, 3, CV_64FC1);
	Mat Rcij(3, 3, CV_64FC1);

	Mat rgij(3, 1, CV_64FC1);
	Mat rcij(3, 1, CV_64FC1);

	double theta_gij;
	double theta_cij;

	Mat rngij(3, 1, CV_64FC1);
	Mat rncij(3, 1, CV_64FC1);

	Mat Pgij(3, 1, CV_64FC1);
	Mat Pcij(3, 1, CV_64FC1);

	Mat tempA(3, 3, CV_64FC1);
	Mat tempb(3, 1, CV_64FC1);

	Mat A;
	Mat b;
	Mat pinA;

	Mat Pcg_prime(3, 1, CV_64FC1);
	Mat Pcg(3, 1, CV_64FC1);
	Mat PcgTrs(1, 3, CV_64FC1);

	Mat Rcg(3, 3, CV_64FC1);
	Mat eyeM = Mat::eye(3, 3, CV_64FC1);

	Mat Tgij(3, 1, CV_64FC1);
	Mat Tcij(3, 1, CV_64FC1);

	Mat tempAA(3, 3, CV_64FC1);
	Mat tempbb(3, 1, CV_64FC1);

	Mat AA;
	Mat bb;
	Mat pinAA;

	Mat Tcg(3, 1, CV_64FC1);

	for (int i = 0; i < nStatus; i++)
	{
		Hgij[i](Rect(0, 0, 3, 3)).copyTo(Rgij);
		Hcij[i](Rect(0, 0, 3, 3)).copyTo(Rcij);

		Rodrigues(Rgij, rgij);
		Rodrigues(Rcij, rcij);

		theta_gij = norm(rgij);
		theta_cij = norm(rcij);

		rngij = rgij / theta_gij;
		rncij = rcij / theta_cij;

		Pgij = 2 * sin(theta_gij / 2)*rngij;
		Pcij = 2 * sin(theta_cij / 2)*rncij;

		tempA = skew(Pgij + Pcij);
		tempb = Pcij - Pgij;

		A.push_back(tempA);
		b.push_back(tempb);
	}

	//Compute rotation
	invert(A, pinA, DECOMP_SVD);

	Pcg_prime = pinA * b;
	Pcg = 2 * Pcg_prime / sqrt(1 + norm(Pcg_prime) * norm(Pcg_prime));
	PcgTrs = Pcg.t();
	Rcg = (1 - norm(Pcg) * norm(Pcg) / 2) * eyeM + 0.5 * (Pcg * PcgTrs + sqrt(4 - norm(Pcg)*norm(Pcg))*skew(Pcg));

	//Computer Translation 
	for (int i = 0; i < nStatus; i++)
	{
		Hgij[i](Rect(0, 0, 3, 3)).copyTo(Rgij);
		Hcij[i](Rect(0, 0, 3, 3)).copyTo(Rcij);
		Hgij[i](Rect(3, 0, 1, 3)).copyTo(Tgij);
		Hcij[i](Rect(3, 0, 1, 3)).copyTo(Tcij);

		tempAA = Rgij - eyeM;
		tempbb = Rcg * Tcij - Tgij;

		AA.push_back(tempAA);
		bb.push_back(tempbb);
	}

	invert(AA, pinAA, DECOMP_SVD);
	Tcg = pinAA * bb;

	Rcg.copyTo(Hcg(Rect(0, 0, 3, 3)));
	Tcg.copyTo(Hcg(Rect(3, 0, 1, 3)));
	Hcg.at<double>(3, 0) = 0.0;
	Hcg.at<double>(3, 1) = 0.0;
	Hcg.at<double>(3, 2) = 0.0;
	Hcg.at<double>(3, 3) = 1.0;

}
// Robot Sensor Calibration: Solving AX = XB on the Euclidean Group
COWACALIB_API int Cowa_Calib::RunHandEyesCalib(const string inputStereoDataFile, float inputRotationAngle, const string outputHandEyesFile)
{
	cv::Mat extrinsicMatricsR_cicj;
	cv::Mat extrinsicMatricsT_cicj;
	cv::Mat rotationMatrics;

	cv::Mat CameraR;
	cv::Mat CameraT;

	Mat A, B, C, D;
	C = (Mat_<double>(1, 4) << 0, 0, 0, 1);
	D = Mat::zeros(3, 1, CV_64FC1);

	//![file_read]
	FileStorage fs(inputStereoDataFile, FileStorage::READ);
	if (!fs.isOpened())
	{
		cout << "Could not open the configuration file: \"" << inputStereoDataFile << "\"" << endl;
		return -1;
	}
	fs["R"] >> extrinsicMatricsR_cicj;
	fs["T"] >> extrinsicMatricsT_cicj;
	//[file_read]

	cv::Mat R_cjci = extrinsicMatricsR_cicj.inv();
	cv::Mat T_cjci = extrinsicMatricsR_cicj.inv() * (-extrinsicMatricsT_cicj);

	//cout << extrinsicMatricsR << endl;
	//cout << extrinsicMatricsT << endl;
	comMatR(R_cjci, T_cjci, A);
	comMatC(A, C, A);

	//旋转矩阵
	rotationMatrics = RotationMatrix(cv::Mat(cv::Vec3d(0, 0, 1)), inputRotationAngle);    //旋转轴的4个角度
	comMatR(rotationMatrics, D, rotationMatrics);
	comMatC(rotationMatrics, C, B);

	vector<Mat> Hgij;
	vector<Mat> Hcij;

	A.convertTo(A, CV_64FC1);
	B.convertTo(B, CV_64FC1);


	Hcij.push_back(A);       //Hcij defines coordinate transformation from Ci to Cj, the camera coordinate system
	Hgij.push_back(B);       //Hcij defines coordinate transformation from Gi to Gj, the gripper coordinate system

	Mat Hcg1(4, 4, CV_64FC1);
	Tsai_HandEye(Hcg1, Hgij, Hcij);

	cout << Hcg1 << endl;

	//Mat H_lc;

	////! [file_read]
	//FileStorage _fs(inputlaserDataFile, FileStorage::READ); // Read the settings
	//if (!_fs.isOpened())
	//{
	//	cout << "Could not open the laserdata file: \"" << inputlaserDataFile << "\"" << endl;
	//	return -1;
	//}

	//_fs["R"] >> CameraR;
	//_fs["T"] >> CameraT;

	//comMatR(CameraR, CameraT, H_lc);
	//comMatC(H_lc, C, H_lc);
	//fs_ << "LaserCamera" << H_lc;

	//! [file_write]
	FileStorage fs_(outputHandEyesFile, FileStorage::WRITE);

	if (fs_.isOpened())
	{
		fs_ << "CameraWorld" << Hcg1;
		fs_.release();
	}
	else
	{
		cout << "Error: can not save the extrinsic parameters\n";
		CV_Assert(false);
	}

	return 0;
}

///P_c  = H_lc * P_l    
///P_w1 = H_cw1 * Pc          // P_w1: Z point to the ground, 15 degree offset from the front. 
///Pw2  = H_w1w2 * P_w1       // P_w2: Z point to the ground. H_w1w2 = rotate(Z, offsetAngle_rad)
///Pw   = H_w2w * P_w2       // P_w: Z point to the heaven. H_w2w = rotate(X, 180)

///2017/07/19
COWACALIB_API int Cowa_Calib::RunTableChange(const string inputBinFile, const string inputHandEyesFile, const string inputLaserCameraFile, float offsetAngle_rad, char bRotate180, const string outputBinFile)
{
	// test by wq
	FILE* fp = fopen(inputBinFile.c_str(), "rb");
	cv::Mat readTranformationMap(481, 641, CV_32FC3);
	float* pDataReadTranformationMap = (float*)readTranformationMap.data;
	if (isLittleEndian())
	{
		fread(pDataReadTranformationMap, 4, readTranformationMap.size().area() * 3, fp);
	}
	else
	{
		char tmpbuf[4];

		for (int i = 0; i < readTranformationMap.size().area() * 3; i++, pDataReadTranformationMap++)
		{
			fread(tmpbuf, 1, 4, fp);
			QUADBYTESSWAP(tmpbuf, (char*)pDataReadTranformationMap);
		}
	}
	fclose(fp);

	Mat H_lc, H_cw1;
	//! [file_read]
	FileStorage fs(inputHandEyesFile, FileStorage::READ); // Read the settings
	if (!fs.isOpened())
	{
		cout << "Could not open the laserdata file: \"" << "out_handeyes_data.xml" << "\"" << endl;
		return -1;
	}
	fs["CameraWorld"] >> H_cw1;

	FileStorage fs2(inputLaserCameraFile, FileStorage::READ); // Read the settings
	if (!fs2.isOpened())
	{
		cout << "Could not open the laserdata file: \"" << "out_handeyes_data.xml" << "\"" << endl;
		return -1;
	}
	fs2["LaserCamera"] >> H_lc;

	Mat H_w1w2 = RotationMatrix(cv::Mat(cv::Vec3d(0, 0, 1)), offsetAngle_rad);
	comMatR(H_w1w2, Mat::zeros(3, 1, CV_64FC1), H_w1w2);
	comMatC(H_w1w2, (Mat_<double>(1, 4) << 0, 0, 0, 1), H_w1w2);

	Mat H_w2w = RotationMatrix(cv::Mat(cv::Vec3d(1, 0, 0)), CV_PI);
	comMatR(H_w2w, Mat::zeros(3, 1, CV_64FC1), H_w2w);
	comMatC(H_w2w, (Mat_<double>(1, 4) << 0, 0, 0, 1), H_w2w);

	// Mat H_lw = H_w2w * H_w1w2 * H_cw1 * H_lc; // bug, it seems in eye-hand calibration, word coordinate is the middle of rotation
	Mat H_lw = H_w2w  * H_cw1 * H_lc;

	// obtain transformation map
	cv::Mat transformMaps(readTranformationMap.rows, readTranformationMap.cols, CV_32FC3);
	for (int y = 0; y < readTranformationMap.rows; y++)
	{
		for (int x = 0; x < readTranformationMap.cols; x++)
		{
			//用二维数组初始化矩阵
			double m[2][1] = { readTranformationMap.at<Vec3f>(y, x)[0], readTranformationMap.at<Vec3f>(y, x)[1] };
			Mat M = Mat(2, 1, CV_64FC1, m);
			Mat C = (Mat_<double>(2, 1) << 0, 1);
			comMatC(M, C, M);

			Mat P_w = H_lw * M; // Z point to the ground

			if (bRotate180)
				transformMaps.at<cv::Vec3f>(480 - y, 640 - x) = cv::Point3f((float)P_w.at<double>(0), (float)P_w.at<double>(1), (float)P_w.at<double>(2));
			else
				transformMaps.at<cv::Vec3f>(y, x) = cv::Point3f((float)P_w.at<double>(0), (float)P_w.at<double>(1), (float)P_w.at<double>(2));
		}
		transformMaps.at<cv::Point3f>(y, transformMaps.cols - 1) = transformMaps.at<cv::Point3f>(y, transformMaps.cols - 2);  //有问题
	}

	//cout << "OK" << endl;
	for (int x = 0; x < transformMaps.cols; x++)
	{
		transformMaps.at<cv::Point3f>(transformMaps.rows - 1, x) = transformMaps.at<cv::Point3f>(transformMaps.rows - 2, x);
	}

	cv::Mat originalTransformationMap;
	transformMaps.copyTo(originalTransformationMap);

	for (int y = 0; y < originalTransformationMap.rows; y++)
	{
		for (int x = 0; x < originalTransformationMap.cols; x++)
		{
			if (originalTransformationMap.at<cv::Vec3f>(y, x)[1] < -100) // y should > -100
			{
				transformMaps.at<cv::Vec3f>(y, x) = cv::Vec3f(-FLT_MAX, -FLT_MAX, -FLT_MAX);
			}
		}
	}

	// save transformaton map; // in little-endian mode
	FILE* fps = fopen(outputBinFile.c_str(), "wb");
	float* pDataTransformationMaps = (float*)transformMaps.data;
	int lWriteSize = 0;
	if (isLittleEndian())
	{
		lWriteSize = fwrite((void*)pDataTransformationMaps, 4, transformMaps.size().area() * 3, fps);
	}
	else // host is big Endian, transform it to little Endian when write to bin file
	{
		char tmpbuf[4];
		for (int i = 0; i < transformMaps.size().area() * 3; i++, pDataTransformationMaps++)
		{
			QUADBYTESSWAP((char*)pDataTransformationMaps, tmpbuf);
			fwrite(tmpbuf, 1, 4, fps);
		}
	}
	fclose(fps);

	/* store all transformation matrix */
	FileStorage fsSave("out_HMatrixs.xml", FileStorage::WRITE); // Read the settings
	if (!fsSave.isOpened())
	{
		cout << "Could not open the laserdata file: \"" << "out_handeyes_data.xml" << "\"" << endl;
		return -1;
	}
	fsSave << "H_lc" << H_lc << "H_cw1" << H_cw1 << "H_lw" << H_lw;
	fsSave.release();


	return 0;
}

cv::Point2f Cowa_Calib::drawCircle(cv::Mat image, cv::Point2f p1, cv::Point2f p2, cv::Point2f& centerPoint)
{
	centerPoint.x = (p1.x + p2.x) / 2;
	centerPoint.y = (p1.y + p2.y) / 2;

	cv::circle(image, centerPoint, 2, cv::Scalar(0, 0, 255));
	return centerPoint;
}

COWACALIB_API std::vector<double> Cowa_Calib::RunComputePara(cv::Mat img, std::vector<double>& para)
{
	Settings s;
	s.boardSize.width = 13;
	s.boardSize.height = 9;

	//if (s.flipVertical)	flip(img, img, 0);
	vector<Point2f> corner;   
	int chessBoardFlags = CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_NORMALIZE_IMAGE;
	bool isFind;

	isFind = findChessboardCorners(img, s.boardSize, corner, chessBoardFlags);
	if (isFind)
	{
		Mat viewGray;
		cvtColor(img, viewGray, COLOR_BGR2GRAY);
		cornerSubPix(viewGray, corner, Size(11, 11),
			Size(-1, -1), TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 30, 0.1));
	}

	cv::Point2f p1, p2;
	p1.x = p1.y = 0;
	p2.x = img.cols;
	p2.y = img.rows;

	cv::Point2f boardCenter;
	cv::Point2f imageCenter;

	drawCircle(img, corner[0], corner[s.boardSize.width * s.boardSize.height - 1], boardCenter);
	drawCircle(img, p1, p2, imageCenter);

	double lrDeviation = boardCenter.x - imageCenter.x;
	double udDeviation = boardCenter.y - imageCenter.y;
	double boardArea = (corner[s.boardSize.width * s.boardSize.height - 1].x - corner[0].x) * (corner[s.boardSize.width * s.boardSize.height - 1].y - corner[0].y);
	double areaDeviation = imgArea - boardArea;     //前后

	para.push_back(lrDeviation);
	para.push_back(udDeviation);
	para.push_back(areaDeviation);

	corner.clear();
	return para;
}