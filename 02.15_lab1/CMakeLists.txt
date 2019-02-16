cmake_minimum_required(VERSION 3.13)

project(qwe)

find_package(OpenCV REQUIRED)

message("opencv version: " ${OpenCV_VERSION})

include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(qwe main.cpp)
target_link_libraries(qwe ${OpenCV_LIBS})
