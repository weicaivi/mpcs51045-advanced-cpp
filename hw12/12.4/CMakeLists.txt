# cmake_minimum_required(VERSION 3.10)
# project(FlexibleTrainFactory VERSION 1.0)

# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_EXTENSIONS OFF)

# include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# set(SOURCES
#     flexible_factory.cpp
# )

# set(HEADERS
#     flexible_factory.h
# )

# add_executable(train_factory ${SOURCES} ${HEADERS})

# if(MSVC)
#     target_compile_options(train_factory PRIVATE /W4)
# else()
#     target_compile_options(train_factory PRIVATE -Wall -Wextra -pedantic)
# endif()

# install(TARGETS train_factory
#     RUNTIME DESTINATION bin
# )

cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 20)
project(flexible_factory)

include_directories(${CMAKE_SOURCE_DIR}/../../fmt/include)

add_executable(flexible_factory flexible_factory.cpp)