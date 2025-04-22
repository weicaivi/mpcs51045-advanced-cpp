cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 20)
project(reference)

include_directories(${CMAKE_SOURCE_DIR}/../../fmt/include)

add_executable(reference reference.cpp)