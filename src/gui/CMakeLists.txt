project(gui)
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(DEBUG_FLAG d)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

#Find Opencv library
find_package(OpenCV REQUIRED)

# Find the Qt library
find_package(Qt5Gui REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5QuickWidgets REQUIRED)
#find_package(Qwt REQUIRED)
#include_directories(${QWT_INCLUDE_DIR})
#link_directories(${QWT_LIBRARY_DIR})
#Find all src files
file(GLOB_RECURSE SRC_FILESE "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")       
#Create code from a list of Qt designer ui files
file(GLOB UI_FILESE "${CMAKE_CURRENT_SOURCE_DIR}/ui/*.ui")       
qt5_wrap_ui( UI_SRCS ${UI_FILESE} )
#Add Qt qrc files
qt5_add_resources(UI_RESOURCES "${CMAKE_CURRENT_SOURCE_DIR}/ui/resources.qrc")  
qt5_add_resources(UI_STYLES "${CMAKE_CURRENT_SOURCE_DIR}/ui/qdarkstyle/style.qrc")  
qt5_add_resources(UI_ICONS "${CMAKE_CURRENT_SOURCE_DIR}/ui/icons.qrc") 
qt5_add_resources(UI_QML "${CMAKE_CURRENT_SOURCE_DIR}/ui/qml.qrc") 

# Tell CMake to create the executable
add_executable(${PROJECT_NAME} WIN32 
			   ${SRC_FILESE} 
			   ${UI_SRCS} 
			   ${UI_RESOURCES} 
			   ${UI_STYLES} 
			   ${UI_ICONS}
			   ${UI_QML}
			   )
# Use the Qt module from Qt 5.
target_link_libraries(${PROJECT_NAME} 
					  Qt5::Gui 
					  Qt5::Core 
					  Qt5::Widgets 
					  Qt5::Network
					  Qt5::QuickWidgets
					  Qt5::Qml
#					  qwt${DEBUG_FLAG}
					  libglog
					  cowaCalib
					  Calib
					  ecatmc
					  fileFunction
					  Image
					  ${EXTRA_LIBS}
					)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin/debug"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin/release"
)

#Use the Opencv module
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})
set_target_properties(${PROJECT_NAME}
   PROPERTIES
   ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
   LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)