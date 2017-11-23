cmake_minimum_required (VERSION 2.6)

cmake_policy (SET CMP0012 NEW)

PROJECT (PR_OpenCL)

set(CLlib CLlib.cpp CLlib.h)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp") 

#Use OpenCV
find_package(OpenCV 3 REQUIRED)
MESSAGE("OpenCV version : ${OpenCV_VERSION}")

include_directories(${OpenCV_INCLUDE_DIRS})
link_directories(${OpenCV_LIBRARY_DIRS})
#MESSAGE("OpenCV Include : ${OpenCV_INCLUDE_DIRS}")
MESSAGE("OpenCV Library : ${OpenCV_LIBRARY_DIRS}")

add_library(Preprocess_Img ${CLlib})
target_link_libraries( Preprocess_Img ${OpenCV_LIBS})
MESSAGE("Linked Library : ${OpenCV_LIBS}")

add_executable( ${PROJECT_NAME} main.cpp CLlib)
target_link_libraries( ${PROJECT_NAME} Preprocess_Img)