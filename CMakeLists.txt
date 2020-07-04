cmake_minimum_required(VERSION 3.0.0)
project(game VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")

find_package(gflags CONFIG REQUIRED)
find_package(glog CONFIG REQUIRED)
find_package(opencv CONFIG REQUIRED)

add_executable(
    raytrace
    main.cpp
)
target_link_libraries(
    raytrace 

    gflags_static
    glog::glog

    opencv_core
    opencv_imgproc
    opencv_imgcodecs
    opencv_highgui
)
