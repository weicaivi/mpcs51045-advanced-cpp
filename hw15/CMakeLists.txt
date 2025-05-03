cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 20)
project(executionPolicies)

find_package(TBB REQUIRED)
add_executable(main main.cpp)
target_link_libraries(main TBB::tbb)