cmake_minimum_required(VERSION 3.9)

project(DirectoryTree)

set(CMAKE_CXX_FLAGS "-std=c++17")
set(CMAKE_BUILD_TYPE DEBUG)

add_executable(DirectoryTree src/main.cpp src/DirectoryTree.cpp)