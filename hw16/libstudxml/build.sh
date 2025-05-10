#!/bin/bash
set -e

# Create build directory
mkdir -p build

# Use clang++ instead of g++ for better Apple Silicon support
echo "Compiling parser.cxx..."
clang++ -std=c++17 -arch arm64 -DHAVE_CONFIG_H -I. -c xml/parser.cxx -o build/parser.o

echo "Compiling serializer.cxx..."
clang++ -std=c++17 -arch arm64 -DHAVE_CONFIG_H -I. -c xml/serializer.cxx -o build/serializer.o

echo "Compiling qname.cxx..."
clang++ -std=c++17 -arch arm64 -DHAVE_CONFIG_H -I. -c xml/qname.cxx -o build/qname.o

echo "Compiling value-traits.cxx..."
clang++ -std=c++17 -arch arm64 -DHAVE_CONFIG_H -I. -c xml/value-traits.cxx -o build/value-traits.o

# Create a static library
echo "Creating static library..."
ar rcs build/libstudxml.a build/*.o

echo "Build completed. Library is in build/libstudxml.a"