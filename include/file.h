#ifndef OPENJA_CPLUSPLUS_FILE_H_
#define OPENJA_CPLUSPLUS_FILE_H_

#include <iostream>
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
		private:
			static int copyFile(const char * pSrc, const char *pDes);
		};
	}
}

#endif