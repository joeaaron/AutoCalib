#ifndef OPENJA_CV_LASER_H_
#define OPENJA_CV_LASER_H_

#include <iostream>
#include <opencv2/opencv.hpp>
#include "config.h"           //preprocessor
/// CV_CALIB_EXPORT
#ifdef CV_IMAGE_EXPORT
#define IMAGE_API __declspec(dllexport)
#else
#define IMAGE_API __declspec(dllimport)
#endif 

using namespace std;

namespace JA{
	namespace CV{
		class IMAGE_API Image
		{
		public:
			static bool ImgAvgGrayValue(cv::Mat img, bool& result);
		};
	}
}

#endif