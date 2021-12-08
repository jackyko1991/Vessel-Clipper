cmake_minimum_required(VERSION 3.11 FATAL_ERROR)
project(cmake-fetchcontent)
set(CMAKE_CXX_STANDARD 11)

message(STATUS "Fetching rapidcsv...")

include(FetchContent)
FetchContent_Declare(
  rapidcsv
  GIT_REPOSITORY "https://github.com/d99kris/rapidcsv.git"
  GIT_TAG        "v8.50"
)

# FetchContent_MakeAvailable(rapidcsv)

FetchContent_GetProperties(rapidcsv)
if(NOT rapidcsv_POPULATED)
  FetchContent_Populate(rapidcsv)
endif()