#ifndef OPENJA_CPLUSPLUS_FILE_H_
#define OPENJA_CPLUSPLUS_FILE_H_

#include <iostream>
#include <vector>
#include "config.h"           //preprocessor

/// CPLUSPLUS_FILE_EXPORT
#ifdef CPLUSPLUS_FILE_EXPORT
#define FILE_API __declspec(dllexport)
#else
#define FILE_API __declspec(dllimport)
#endif 

using namespace std;

namespace JA{
	namespace CPLUSPLUS{
		class FILE_API File
		{
		public:
			static int copyDir(const char* pSrc, const char* pDes);
			static int countDirNum(const char* pSrc, int& dirNum);
			static void getAllFiles(string pSrcPath, vector<string>& files);
			static void getAllFormatFiles(string path, vector<string>& files, string format);
			static void delAllFormatFiles(string path);
			static void delAllFiles(string path);
		private:
			static int copyFile(const char * pSrc, const char *pDes);
			
		};
	}
}

#endif