#ifndef OPENJA_CV_CALIB_H_
#define OPENJA_CV_CALIB_H_

#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include "config.h"           //preprocessor
/// CV_CALIB_EXPORT
#ifdef CV_CALIB_EXPORT
#define  CALIB_API __declspec(dllexport)
#else
#define CALIB_API __declspec(dllimport)
#endif 

using namespace std;

namespace JA{
	namespace CV{
		class CALIB_API Calib
		{
		public:
			int RunCalibrateCamera(const std::string inputSettingsFile, const std::string outCameraDataFilePath, cv::Mat* prefCameraMatrix = NULL, cv::Mat* prefDistCoefficent = NULL);
			int RunCalibrateLaser(const std::string inputCameraDataFile, const std::string inputImageListFile, std::string outputLaserCameraFile, const std::string outputBinFile);
			int RunStereoCalib(const std::string inputCameraDataFile, const std::string inputSettingsFile, const std::string outStereoDataFilePath);
			int RunHandEyesCalib(const std::string inputStereoDataFile, float inputRotationAngle, const std::string outputHandEyesFile);
			int RunTableChange(const std::string inputBinFile, const std::string inputHandEyesFile, const std::string inputLaserCameraFile, float offsetAngle_rad, char bRotate180, const std::string outputBinFile, char bCheckWorldCoordinate = 1);
			std::vector<double> GetDeviationPara(cv::Mat, vector<double>& );
		private:
			void sortConnerPoints(std::vector<cv::Point2f>& corners);
		};
	}
}

#endif