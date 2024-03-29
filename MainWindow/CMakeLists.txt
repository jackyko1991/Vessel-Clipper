cmake_minimum_required(VERSION 3.1)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 11)

get_filename_component(
	LIBRARY_NAME 
	${CMAKE_CURRENT_SOURCE_DIR} 
	NAME
)

project(${LIBRARY_NAME})

message(
	STATUS 
	"Configuring module ${LIBRARY_NAME}..."
)

# # # # # # # # # # # # # # # # # Dependency # # # # # # # # # # # # # # # #  
# VMTK
find_package(VMTK REQUIRED)
include(${VMTK_USE_FILE})

# ITK
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

# VTK
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

# QT
find_package(Qt5 COMPONENTS Core Widgets Concurrent PrintSupport REQUIRED)

# Custom libraries
include_directories(${Core_SOURCE_DIR})
include_directories(${CenterlinesInfoWidget_SOURCE_DIR})
include_directories(${Branch_SOURCE_DIR})

# Third party dependency
include(FetchContent)
FetchContent_Declare(
  rapidcsv
  GIT_REPOSITORY "https://github.com/d99kris/rapidcsv.git"
  GIT_TAG        "v8.50"
)
if(NOT rapidcsv_POPULATED)
  FetchContent_Populate(rapidcsv)
  include_directories(${rapidcsv_SOURCE_DIR}/src)
endif()


# Finding needed source files.
file(
	GLOB
	LIBRARY_SRC
	*.c*
)
# Finding needed header files.
file(
	GLOB
	LIBRARY_INC
	*.h*
)
# Finding needed ui files.
file(
	GLOB 
	LIBRARY_UI 
	*.ui
)

# # # # # # # # # # # # # # # # # Qt UI wrapping # # # # # # # # # # # # # # # #  
qt5_wrap_ui(
	LIBRARY_UI_H
	${LIBRARY_UI}
)

qt5_wrap_cpp(
    LIBRARY_MOC
    ${LIBRARY_INC}
)

qt5_add_resources(
	LIBRARY_QRC_SRC
	${LIBRARY_QRC}
)

# # # # # # # # # # # # # # # # # BUILD LIBRARY # # # # # # # # # # # # # # # #  
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
set(LIB_TYPE SHARED)

add_library(
	${LIBRARY_NAME}
	${LIB_TYPE}
	${LIBRARY_SRC}
	${LIBRARY_INC}
	${LIBRARY_UI_H}
	${LIBRARY_MOC}
	${LIBRARY_QRC_SRC}
)

target_link_libraries(
	${LIBRARY_NAME}
	Core
	CenterlinesInfoWidget
	Branch
	${VTK_LIBRARIES}
	${VMTK_LIBRARIES}
)
qt5_use_modules(${LIBRARY_NAME} Core Widgets Concurrent PrintSupport)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

#Grouping in MSVC
source_group(
	"Generated Files"
	FILES
	${LIBRARY_MOC}
	${LIBRARY_UI_H}
	${LIBRARY_QRC_SRC}
)
source_group(
	"Ui Files"
	FILES
	${LIBRARY_UI}
)
source_group(
	"Header Files" 
	FILES 
	${LIBRARY_INC}
)
source_group(
	"Source Files" 
	FILES 
	${LIBRARY_SRC}
)

MESSAGE(STATUS "Configuring module ${LIBRARY_NAME} finished!")
