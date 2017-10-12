cmake_minimum_required(VERSION 2.8.11)
project(COWA-AUTO-CALIB)
include(CMakeDependentOption)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
# The version number.
set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)
set(VERSION_PATCH 0)
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/include/config.h.in"
  "${PROJECT_SOURCE_DIR}/include/config.h"
  )
# include external library header files
include_directories("${PROJECT_SOURCE_DIR}/include"
                    "${PROJECT_SOURCE_DIR}/src/ecatmc")
# link external library directories
link_directories("${PROJECT_SOURCE_DIR}/lib")

# select library or target to build 
option(BUILD_CV			"Build Computer Vision" 	ON)
option(BUILD_GUI		"Build Graphic Interfaces"	ON)
cmake_dependent_option(CV_CALIB_EXPORT	"build shared cowaCalib library"	OFF
					   "BUILD_CV" ON)	
#set a variable for adding module
#set(EXTRA_LIBS "ecatmc")
set(EXTRA LIBS "cowaCalib")
set(EXTRA_LIBS "Calib")
set(EXTRA_LIBS "File")

if(BUILD_CV)
	add_subdirectory(src/cv)
endif(BUILD_CV)

if(BUILD_GUI)
	add_subdirectory(src/gui)
endif(BUILD_GUI)

add_subdirectory(src/ecatmc)
