cmake_minimum_required(VERSION 3.13)

project(grey_lvl_threshold)

find_package(OpenCV REQUIRED)

message("opencv version: " ${OpenCV_VERSION})

include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} pthread)
