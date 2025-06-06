cmake_minimum_required(VERSION 3.20)

# Project configuration
project(DistributedTaskProcessor 
    VERSION 1.0.0
    DESCRIPTION "Distributed Task Processing Framework"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler-specific flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    add_compile_options(
        -Wall -Wextra -Wpedantic
        -Wno-unused-parameter
        -fdiagnostics-color=always
        -fconcepts
    )
    
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(
        -Wall -Wextra -Wpedantic
        -Wno-unused-parameter
        -fdiagnostics-color=always
        -stdlib=libc++
    )
    
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options(
        /W4 /permissive- 
        /std:c++latest
        /experimental:module
    )
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Find required packages
find_package(Threads REQUIRED)

# Find TBB for enhanced parallel execution
find_package(TBB QUIET)
if(TBB_FOUND)
    add_compile_definitions(DTPF_USE_TBB)
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(DTPF_SOURCES
    meta_programming.cpp
    factory_pattern.cpp
    config_parser.cpp
    concurrency.cpp
    execution_engine.cpp
    main_framework.cpp
)

set(DTPF_HEADERS)

add_executable(dtpf_framework ${DTPF_SOURCES})

target_link_libraries(dtpf_framework 
    PUBLIC 
        Threads::Threads
)

if(TBB_FOUND)
    target_link_libraries(dtpf_framework PUBLIC TBB::tbb)
endif()

# Target properties
set_target_properties(dtpf_framework PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
)

install(TARGETS dtpf_framework
    RUNTIME DESTINATION bin
)

find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
    set(DOXYGEN_EXTRACT_ALL YES)
    set(DOXYGEN_BUILTIN_STL_SUPPORT YES)
    set(DOXYGEN_USE_MDFILE_AS_MAINPAGE README.md)
    
    doxygen_add_docs(docs
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/README.md
        COMMENT "Generate documentation"
    )
endif()

# Custom targets for code quality
add_custom_target(format
    COMMAND clang-format -i ${DTPF_SOURCES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Format source code"
)

add_custom_target(tidy
    COMMAND clang-tidy ${DTPF_SOURCES} -- -std=c++23
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Run static analysis"
)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
if(TBB_FOUND)
    message(STATUS "TBB support: Enabled")
else()
    message(STATUS "TBB support: Disabled")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "12.0")
        message(WARNING "GCC < 12.0 may have limited std::execution support")
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "15.0")
        message(WARNING "Clang < 15.0 may have limited std::execution support")
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.30")
        message(WARNING "MSVC < 19.30 may have limited std::execution support")
    endif()
endif()