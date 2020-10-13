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
# VTK
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

# QT
find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

# JSON
if(${CMAKE_VERSION} VERSION_LESS "3.11.0") 
    find_package(nlohmann_json 3.2.0 REQUIRED)
else(${CMAKE_VERSION} VERSION_LESS "3.11.0")
    message(STATUS "Fetching nlohmann_json...")
    include(FetchContent)

    FetchContent_Declare(json
      GIT_REPOSITORY https://github.com/nlohmann/json.git
      GIT_TAG v3.7.3)

    FetchContent_GetProperties(json)
    if(NOT json_POPULATED)
      FetchContent_Populate(json)
      add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
    endif()
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
    ${VTK_LIBRARIES}
    nlohmann_json
)

#target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

qt5_use_modules(${LIBRARY_NAME} Core Widgets)

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
