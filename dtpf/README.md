# Distributed Task Processing Framework (DTPF)

## Overview

This project showcases C++ concepts including meta-programming, factory patterns, regular expressions, concurrency, and modern C++ features. The framework allows you to create, configure, and execute different types of tasks using various execution strategies.

## Features

### **Advanced C++ Techniques**
- **Meta-programming**: Concepts, templates, SFINAE
- **Factory patterns**: Dynamic task creation and registration
- **Regular expressions**: Configuration parsing
- **Concurrency**: Parallel execution with `std::async` and `std::future`
- **Modern C++**: Smart pointers, RAII, move semantics

### **Task Types**
- **DataProcessingTask**: Simulates CPU-intensive data processing
- **NetworkTask**: Simulates network operations with configurable timeouts
- **ComputationTask**: Mathematical computations (Fibonacci, factorial, prime counting)

### **Execution Strategies**
- **Sequential**: Tasks execute one after another
- **Parallel**: Tasks execute simultaneously using multiple threads
- **Pipeline**: Tasks execute in sequence with data flow
- **Adaptive**: Automatically chooses the best strategy based on task characteristics

## Building and Running

### Prerequisites
- C++23 compatible compiler (GCC 12+, Clang 15+, or MSVC 19.30+)
- CMake 3.20+
- pthread support

### Build Instructions
```bash
cd dtpf

mkdir build && cd build

cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

./dtpf_framework
```

## Usage Example

```cpp
#include "dtpf/task_framework.hpp"

int main() {
    using namespace dtpf;
    
    DistributedTaskProcessor processor;
    
    processor.create_and_add_task<DataProcessingTask>("my_data", 2, 8);
    processor.create_and_add_task<ComputationTask>(20, "fibonacci");
    processor.create_and_add_task<NetworkTask>("https://api.example.com", 1000);
    
    processor.set_execution_strategy(ExecutionStrategy::Parallel);
    
    auto results = processor.execute_all_tasks();
    
    for (const auto& result : results) {
        std::cout << "Result: " << result << std::endl;
    }
    
    return 0;
}
```

## Configuration Support

The framework supports text-based configuration using regular expressions:

```
task: DataProcessing {
    input = "large_dataset"
    multiplier = "3"
    priority = "9"
}

task: Computation {
    iterations = "25"
    algorithm = "fibonacci"
}

task: Network {
    url = "https://service.example.com/api"
    timeout = "2000"
}
```

## Performance Features

- **Thread pooling** for efficient resource management
- **Work-stealing** algorithms for load balancing
- **Priority-based scheduling** for task execution
- **Performance monitoring** with execution timing
- **Adaptive execution** based on task characteristics


## Sample Output

```
=== Distributed Task Processing Framework Demo ===

1. Manual Task Creation Example:

=== Task Summary ===
Total tasks: 4
Task types:
  Computation: 1
  DataProcessing: 2
  Network: 1
==================

Executing with PARALLEL strategy:
Executing 4 tasks...
Execution completed in 254ms

Results:
Task 1: sample_data_1_processed_processed:26:1748310188
Task 2: Response from https://api.example.com:37:1748310188
Task 3: fibonacci_result_610:610:1748310188
Task 4: sample_data_2_processed:13:1748310188

2. Strategy Comparison Example:

Testing SEQUENTIAL strategy:
Executing 3 tasks...
Execution completed in 307ms
Strategy: SEQUENTIAL - Duration: 307ms

Testing PARALLEL strategy:
Executing 3 tasks...
Execution completed in 155ms
Strategy: PARALLEL - Duration: 155ms

=== Demo completed successfully! ===
```
