# Use clang++ on macOS
CXX = clang++
# Use C++17 standard and specify arm64 architecture
CXXFLAGS = -std=c++17 -Wall -Wextra -arch arm64
# Include paths
INCLUDES = -I. -I../libstudxml

# Define the target executable
TARGET = xsd2cpp

# Enable verbose output for debugging
VERBOSE = 1

all: $(TARGET)

# Explicit compilation rules
driver.o: driver.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

IndentStream.o: IndentStream.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Link rule with direct path to the library file
$(TARGET): driver.o IndentStream.o
	$(CXX) $(CXXFLAGS) -o $@ $^ ../libstudxml/build/libstudxml.a

# Clean rule
clean:
	rm -f driver.o IndentStream.o $(TARGET)

.PHONY: all clean