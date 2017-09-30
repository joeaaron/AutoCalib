#ifndef _ECAT_IMAGETHIN_H_
#define _ECAT_IMAGETHIN_H_

#include "config.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>

#ifdef  CV_CALIB_EXPORT
#define COWACALIB_API __declspec(dllexport)
#else
#define COWACALIB_API __declspec(dllimport)
#endif // CV_IMAGETHIN_EXPORT

using namespace cv;
using namespace std;

cv::Mat cameraMatrix;
cv::Mat distcofficients;
cv::Mat undistorMapX, undistorMapY;

cv::Mat distortImagesPoints;
cv::Mat imagesPoints;
cv::Mat extrinsicMatrixs;

cv::Mat* imgList;
cv::Mat* imgListUndistort;
cv::Mat* imgBinary;

cv::Mat laserPoints2f;
cv::Mat laserPoints_CameraCoord3d;

int nFrame;
int board_width;
int board_height;
float square_size;
int validLaserPointCount;

static class Settings
{
public:
	Settings() : goodInput(false) {}
	enum Pattern { NOT_EXISTING, CHESSBOARD, CIRCLES_GRID, ASYMMETRIC_CIRCLES_GRID };
	enum InputType { INVALID, CAMERA, VIDEO_FILE, IMAGE_LIST };

	void write(FileStorage& fs) const                        //Write serialization for this class
	{
		fs << "{"
			<< "BoardSize_Width" << boardSize.width
			<< "BoardSize_Height" << boardSize.height
			<< "Square_Size" << squareSize
			<< "Calibrate_Pattern" << patternToUse
			<< "Calibrate_NrOfFrameToUse" << nrFrames
			<< "Calibrate_FixAspectRatio" << aspectRatio
			<< "Calibrate_AssumeZeroTangentialDistortion" << calibZeroTangentDist
			<< "Calibrate_FixPrincipalPointAtTheCenter" << calibFixPrincipalPoint

			<< "Write_DetectedFeaturePoints" << writePoints
			<< "Write_extrinsicParameters" << writeExtrinsics
			<< "Write_outputFileName" << outputFileName

			<< "Show_UndistortedImage" << showUndistorsed

			<< "Input_FlipAroundHorizontalAxis" << flipVertical
			<< "Input_Delay" << delay
			<< "Input" << input
			<< "}";
	}
	void read(const FileNode& node)                          //Read serialization for this class
	{
		node["BoardSize_Width"] >> boardSize.width;
		node["BoardSize_Height"] >> boardSize.height;
		node["Calibrate_Pattern"] >> patternToUse;
		node["Square_Size"] >> squareSize;
		node["Calibrate_NrOfFrameToUse"] >> nrFrames;
		node["Calibrate_FixAspectRatio"] >> aspectRatio;
		node["Write_DetectedFeaturePoints"] >> writePoints;
		node["Write_extrinsicParameters"] >> writeExtrinsics;
		node["Write_outputFileName"] >> outputFileName;
		node["Calibrate_AssumeZeroTangentialDistortion"] >> calibZeroTangentDist;
		node["Calibrate_FixPrincipalPointAtTheCenter"] >> calibFixPrincipalPoint;
		node["Calibrate_UseFisheyeModel"] >> useFisheye;
		node["Input_FlipAroundHorizontalAxis"] >> flipVertical;
		node["Show_UndistortedImage"] >> showUndistorsed;
		node["Input"] >> input;
		node["Input_Delay"] >> delay;
		validate();
	}
	void validate()
	{
		goodInput = true;
		if (boardSize.width <= 0 || boardSize.height <= 0)
		{
			cerr << "Invalid Board size: " << boardSize.width << " " << boardSize.height << endl;
			goodInput = false;
		}
		if (squareSize <= 10e-6)
		{
			cerr << "Invalid square size " << squareSize << endl;
			goodInput = false;
		}
		if (nrFrames <= 0)
		{
			cerr << "Invalid number of frames " << nrFrames << endl;
			goodInput = false;
		}

		if (input.empty())      // Check for valid input
			inputType = INVALID;
		else
		{
			if (input[0] >= '0' && input[0] <= '9')
			{
				stringstream ss(input);
				ss >> cameraID;
				inputType = CAMERA;
			}
			else
			{
				if (readStringList(input, imageList))
				{
					inputType = IMAGE_LIST;
					nrFrames = (nrFrames < (int)imageList.size()) ? nrFrames : (int)imageList.size();
				}
				else
					inputType = VIDEO_FILE;
			}
			if (inputType == CAMERA)
				inputCapture.open(cameraID);
			if (inputType == VIDEO_FILE)
				inputCapture.open(input);
			if (inputType != IMAGE_LIST && !inputCapture.isOpened())
				inputType = INVALID;
		}
		if (inputType == INVALID)
		{
			cerr << " Input does not exist: " << input;
			goodInput = false;
		}

		calibrationPattern = NOT_EXISTING;
		if (!patternToUse.compare("CHESSBOARD")) calibrationPattern = CHESSBOARD;
		if (!patternToUse.compare("CIRCLES_GRID")) calibrationPattern = CIRCLES_GRID;
		if (!patternToUse.compare("ASYMMETRIC_CIRCLES_GRID")) calibrationPattern = ASYMMETRIC_CIRCLES_GRID;
		if (calibrationPattern == NOT_EXISTING)
		{
			cerr << " Camera calibration mode does not exist: " << patternToUse << endl;
			goodInput = false;
		}
		atImageList = 0;

	}
	Mat nextImage()
	{
		Mat result;
		if (inputCapture.isOpened())
		{
			Mat view0;
			inputCapture >> view0;
			view0.copyTo(result);
		}
		else if (atImageList < imageList.size())
			result = imread(imageList[atImageList++], IMREAD_COLOR);

		return result;
	}

	static bool readStringList(const string& filename, vector<string>& l)
	{
		l.clear();
		FileStorage fs(filename, FileStorage::READ);
		if (!fs.isOpened())
			return false;
		FileNode n = fs.getFirstTopLevelNode();
		if (n.type() != FileNode::SEQ)
			return false;
		FileNodeIterator it = n.begin(), it_end = n.end();
		for (; it != it_end; ++it)
			l.push_back((string)*it);
		return true;
	}
public:
	Size boardSize;              // The size of the board -> Number of items by width and height
	Pattern calibrationPattern;  // One of the Chessboard, circles, or asymmetric circle pattern
	float squareSize;            // The size of a square in your defined unit (point, millimeter,etc).
	int nrFrames;                // The number of frames to use from the input for calibration
	float aspectRatio;           // The aspect ratio
	int delay;                   // In case of a video input
	bool writePoints;            // Write detected feature points
	bool writeExtrinsics;        // Write extrinsic parameters
	bool calibZeroTangentDist;   // Assume zero tangential distortion
	bool calibFixPrincipalPoint; // Fix the principal point at the center
	bool flipVertical;           // Flip the captured images around the horizontal axis
	string outputFileName;       // The name of the file where to write
	bool showUndistorsed;        // Show undistorted images after calibration
	string input;                // The input ->
	bool useFisheye;             // use fisheye camera model for calibration

	int cameraID;
	vector<string> imageList;
	size_t atImageList;
	VideoCapture inputCapture;
	InputType inputType;
	bool goodInput;
	int flag;

private:
	string patternToUse;

};

class DistortF :public cv::MinProblemSolver::Function
{
public:
	DistortF(double _distortU, double _distortV) : distortU(_distortU), distortV(_distortV)
	{
		const double* const distPtr = distcofficients.ptr<double>();

		inverseCameraMatrix = (cameraMatrix.colRange(0, 3)).inv(DECOMP_LU);

		k1 = distPtr[0];
		k2 = distPtr[1];
		p1 = distPtr[2];
		p2 = distPtr[3];
		k3 = distcofficients.cols + distcofficients.rows - 1 >= 5 ? distPtr[4] : 0.;
		k4 = distcofficients.cols + distcofficients.rows - 1 >= 8 ? distPtr[5] : 0.;
		k5 = distcofficients.cols + distcofficients.rows - 1 >= 8 ? distPtr[6] : 0.;
		k6 = distcofficients.cols + distcofficients.rows - 1 >= 8 ? distPtr[7] : 0.;
		s1 = distcofficients.cols + distcofficients.rows - 1 >= 12 ? distPtr[8] : 0.;
		s2 = distcofficients.cols + distcofficients.rows - 1 >= 12 ? distPtr[9] : 0.;
		s3 = distcofficients.cols + distcofficients.rows - 1 >= 12 ? distPtr[10] : 0.;
		s4 = distcofficients.cols + distcofficients.rows - 1 >= 12 ? distPtr[11] : 0.;
		tauX = distcofficients.cols + distcofficients.rows - 1 >= 14 ? distPtr[12] : 0.;
		tauY = distcofficients.cols + distcofficients.rows - 1 >= 14 ? distPtr[13] : 0.;
	}

	int getDims() const { return 2; }
	double calc(const double* x)const{

		double undistortU = x[0];
		double undistortV = x[1];
		const double* ir = &inverseCameraMatrix(0, 0);
		const double* pA = (const double*)cameraMatrix.data;

		double Xd = undistortU * ir[0] + undistortV * ir[1] + ir[2], Yd = undistortU * ir[3] + undistortV * ir[4] + ir[5], Wd = undistortU * ir[6] + undistortV * ir[7] + ir[8];
		Wd = 1. / Wd;
		Xd = Xd * Wd;
		Yd = Yd * Wd;

		double Xd_2 = Xd*Xd, Yd_2 = Yd * Yd, r_2 = Xd_2 + Yd_2, _2XdYd = 2 * Xd * Yd;
		double kr = (1 + ((k3*r_2 + k2)*r_2 + k1)*r_2) / (1 + ((k6*r_2 + k5)*r_2 + k4)*r_2);
		double Xdd = (Xd*kr + p1*_2XdYd + p2*(r_2 + 2 * Xd_2) + s1*r_2 + s2*r_2*r_2);;
		double Ydd = (Yd*kr + p1*(r_2 + 2 * Yd_2) + p2*_2XdYd + s3*r_2 + s4*r_2*r_2);
		double Wdd = Wd;

		double distortU_d = pA[0] * Xdd + pA[1] * Ydd + pA[2] * Wdd;
		double distortV_d = pA[3] * Xdd + pA[4] * Ydd + pA[5] * Wdd;

		return sqrt((distortU - distortU_d) * (distortU - distortU_d) + (distortV - distortV_d) * (distortV - distortV_d));
	}
private:
	double distortU, distortV;

	double k1;
	double k2;
	double p1;
	double p2;
	double k3;
	double k4;
	double k5;
	double k6;
	double s1;
	double s2;
	double s3;
	double s4;
	double tauX;
	double tauY;

	Mat_<double> inverseCameraMatrix;
};

namespace ECAT {
	namespace  CALIB {
		class COWACALIB_API Cowa_Calib
		{
		public:
			 int RunCalibrateCamera(const string inputSettingsFile, const string outCameraDataFilePath);
			 std::vector<double> RunComputePara(cv::Mat img, vector<double>& paras);
			 int RunCalibrateLaser(const string inputCameraDataFile, const string inputImageListFile, const string outputLaserCameraFile, const string outputBinFile);
			 int RunStereoCalib(const string inputCameraDataFile, const string inputSettingsFile, const string outStereoDataFilePath);
			 int RunHandEyesCalib(const std::string inputStereoDataFile, float inputRotationAngle, const std::string outputHandEyesFile);
			 int RunTableChange(const std::string inputBinFile, const std::string inputHandEyesFile, const std::string inputLaserCameraFile, float offsetAngle_rad, char bRotate180, const std::string outputBinFile);
		private:
			bool runCalibrationAndSave(Settings& s, Size imageSize, Mat& cameraMatrix, Mat& distCoeffs,
			vector<vector<Point2f> > imagePoints);
			bool runCalibration(Settings& s, Size& imageSize, Mat& cameraMatrix, Mat& distCoeffs,
				vector<vector<Point2f> > imagePoints, vector<Mat>& rvecs, vector<Mat>& tvecs,
				vector<float>& reprojErrs, double& totalAvgErr);
			double computeReprojectionErrors(const vector<vector<Point3f> >& objectPoints,
				const vector<vector<Point2f> >& imagePoints,
				const vector<Mat>& rvecs, const vector<Mat>& tvecs,
				const Mat& cameraMatrix, const Mat& distCoeffs,
				vector<float>& perViewErrors, bool fisheye);
			void calcBoardCornerPositions(Size boardSize, float squareSize, vector<Point3f>& corners,
				Settings::Pattern patternType /*= Settings::CHESSBOARD*/);
			void saveCameraParams(Settings& s, Size& imageSize, Mat& cameraMatrix, Mat& distCoeffs,
				const vector<Mat>& rvecs, const vector<Mat>& tvecs,
				const vector<float>& reprojErrs, const vector<vector<Point2f> >& imagePoints,
				double totalAvgErr);

			void getAndSaveTransformation(cv::Mat LaserPlane, std::string outputLaserCameraFile, std::string outputBinFile);
			cv::Mat getNearestPlanePoint(cv::Vec4d plane, cv::Vec3d point);
			cv::Mat getRotationMatix(cv::Mat originVec, cv::Mat expectedVec);
			Mat comMatC(Mat Matrix1, Mat Matrix2, Mat &MatrixCom);
			Mat comMatR(Mat Matrix1, Mat Matrix2, Mat &MatrixCom);
			void QUADBYTESSWAP(char src[4], char dst[4]);
			bool isLittleEndian();

			void calRealPoint(vector<vector<Point3f>>& obj, Settings& s);
			void outputCameraParam(Settings& s);

			Mat RotationMatrix(Mat& axis, const double& angleRad);
			void Tsai_HandEye(Mat Hcg, vector<Mat> Hgij, vector<Mat> Hcij);
			Mat skew(Mat A);
			cv::Point2f drawCircle(cv::Mat image, Point2f p1, Point2f p2, cv::Point2f& centerPoint);

		private:
			Mat R, T, E, F;                                         //R 旋转矢量 T平移矢量 E本征矩阵 F基础矩阵  
			vector<Mat> rvecs;                                      //旋转向量  
			vector<Mat> tvecs;                                      //平移向量  
			vector<vector<Point2f>> imagePointL;                    //左边摄像机所有照片角点的坐标集合  
			vector<vector<Point2f>> imagePointR;                    //右边摄像机所有照片角点的坐标集合  
			vector<vector<Point3f>> objRealPoint;                   //各副图像的角点的实际物理坐标集合  

			vector<Point2f> cornerL;								//左边摄像机某一照片角点坐标集合  
			vector<Point2f> cornerR;								//右边摄像机某一照片角点坐标集合  

			Mat rgbImageL, grayImageL;
			Mat rgbImageR, grayImageR;

			Mat Rl, Rr, Pl, Pr, Q;                                  //校正旋转矩阵R，投影矩阵P 重投影矩阵Q 
			Mat mapLx, mapLy, mapRx, mapRy;                         //映射表  
			Rect validROIL, validROIR;                              //图像校正之后，会对图像进行裁剪，这里的validROI就是指裁剪之后的区域  

			Mat cameraMatrixL, cameraMatrixR;
			Mat distCoeffL, distCoeffR;

		};

	}
}


#endif