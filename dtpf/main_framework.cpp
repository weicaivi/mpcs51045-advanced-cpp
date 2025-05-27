// main_framework.cpp
// Main framework integration and example task implementations

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <thread>
#include <chrono>
#include <future>
#include <functional>
#include <stdexcept>
#include <set>

namespace dtpf {

// ============================================================================
// RESULT TYPE - DEFINED ONCE
// ============================================================================

struct ProcessingResult {
    std::string data;
    int processed_count = 0;
    std::chrono::system_clock::time_point timestamp;
    
    ProcessingResult() : timestamp(std::chrono::system_clock::now()) {}
    ProcessingResult(const std::string& d, int count) : data(d), processed_count(count), timestamp(std::chrono::system_clock::now()) {}
    
    std::string serialize() const {
        auto time_t = std::chrono::system_clock::to_time_t(timestamp);
        return data + ":" + std::to_string(processed_count) + ":" + std::to_string(time_t);
    }
    
    static ProcessingResult deserialize(const std::string& str) {
        size_t first_colon = str.find(':');
        size_t second_colon = str.find(':', first_colon + 1);
        if (first_colon != std::string::npos && second_colon != std::string::npos) {
            ProcessingResult result;
            result.data = str.substr(0, first_colon);
            result.processed_count = std::stoi(str.substr(first_colon + 1, second_colon - first_colon - 1));
            auto time_t = std::stoi(str.substr(second_colon + 1));
            result.timestamp = std::chrono::system_clock::from_time_t(time_t);
            return result;
        }
        return ProcessingResult{str, 0};
    }
};

// ============================================================================
// BASE CLASSES - DEFINED ONCE
// ============================================================================

class TaskBase {
public:
    virtual ~TaskBase() = default;
    virtual std::string execute() = 0;
    virtual std::string get_type() const = 0;
    virtual int get_priority() const = 0;
};

template<typename T>
concept TaskResult = requires(T t) {
    { t.serialize() } -> std::convertible_to<std::string>;
    { T::deserialize(std::string{}) } -> std::same_as<T>;
    std::movable<T>;
};

template<TaskResult R>
class Task : public TaskBase {
public:
    virtual R execute_typed() = 0;
    std::string execute() override {
        return execute_typed().serialize();
    }
};

// ============================================================================
// ALL TASK CLASSES - DEFINED ONCE
// ============================================================================

class ComputationTask : public Task<ProcessingResult> {
public:
    ComputationTask(int iterations, const std::string& algorithm = "fibonacci") 
        : iterations_(iterations), algorithm_(algorithm) {}
    
    ProcessingResult execute_typed() override {
        int result = 0;
        if (algorithm_ == "fibonacci") {
            result = fibonacci(iterations_);
        } else if (algorithm_ == "factorial") {
            result = factorial(iterations_);
        } else if (algorithm_ == "prime_count") {
            result = count_primes(iterations_);
        }
        std::string result_data = algorithm_ + "_result_" + std::to_string(result);
        return ProcessingResult{result_data, result};
    }
    
    std::string get_type() const override { return "Computation"; }
    int get_priority() const override { return 8; }
    
    static ComputationTask from_config(const std::string& config) {
        int iterations = 10;
        std::string algorithm = "fibonacci";
        if (config.find("iterations=") != std::string::npos) {
            size_t start = config.find("iterations=") + 11;
            size_t end = config.find(';', start);
            iterations = std::stoi(config.substr(start, end - start));
        }
        if (config.find("algorithm=") != std::string::npos) {
            size_t start = config.find("algorithm=") + 10;
            size_t end = config.find(';', start);
            if (end == std::string::npos) end = config.length();
            algorithm = config.substr(start, end - start);
        }
        return ComputationTask{iterations, algorithm};
    }
    
private:
    int iterations_;
    std::string algorithm_;
    
    int fibonacci(int n) {
        if (n <= 1) return n;
        int a = 0, b = 1;
        for (int i = 2; i <= n; ++i) {
            int temp = a + b;
            a = b;
            b = temp;
        }
        return b;
    }
    
    int factorial(int n) {
        if (n <= 1) return 1;
        int result = 1;
        for (int i = 2; i <= n; ++i) {
            result *= i;
        }
        return result;
    }
    
    int count_primes(int limit) {
        if (limit < 2) return 0;
        int count = 0;
        for (int i = 2; i <= limit; ++i) {
            bool is_prime = true;
            for (int j = 2; j * j <= i; ++j) {
                if (i % j == 0) {
                    is_prime = false;
                    break;
                }
            }
            if (is_prime) count++;
        }
        return count;
    }
};

class DataProcessingTask : public Task<ProcessingResult> {
public:
    DataProcessingTask(const std::string& input_data, int multiplier = 1, int priority = 5) 
        : input_data_(input_data), multiplier_(multiplier), priority_(priority) {}
    
    ProcessingResult execute_typed() override {
        std::this_thread::sleep_for(std::chrono::milliseconds(100 + (multiplier_ * 50)));
        std::string processed_data = input_data_;
        for (int i = 0; i < multiplier_; ++i) {
            processed_data += "_processed";
        }
        int count = static_cast<int>(input_data_.length()) * multiplier_;
        return ProcessingResult{processed_data, count};
    }
    
    std::string get_type() const override { return "DataProcessing"; }
    int get_priority() const override { return priority_; }
    
    static DataProcessingTask from_config(const std::string& config) {
        std::string input = "default_data";
        int multiplier = 1;
        int priority = 5;
        
        if (config.find("input=") != std::string::npos) {
            size_t start = config.find("input=") + 6;
            size_t end = config.find(';', start);
            input = config.substr(start, end - start);
        }
        if (config.find("multiplier=") != std::string::npos) {
            size_t start = config.find("multiplier=") + 11;
            size_t end = config.find(';', start);
            multiplier = std::stoi(config.substr(start, end - start));
        }
        if (config.find("priority=") != std::string::npos) {
            size_t start = config.find("priority=") + 9;
            size_t end = config.find(';', start);
            if (end == std::string::npos) end = config.length();
            priority = std::stoi(config.substr(start, end - start));
        }
        return DataProcessingTask{input, multiplier, priority};
    }
    
private:
    std::string input_data_;
    int multiplier_;
    int priority_;
};

class NetworkTask : public Task<ProcessingResult> {
public:
    NetworkTask(const std::string& url, int timeout_ms = 1000) 
        : url_(url), timeout_ms_(timeout_ms) {}
    
    ProcessingResult execute_typed() override {
        std::this_thread::sleep_for(std::chrono::milliseconds(timeout_ms_ / 2));
        std::string response_data = "Response from " + url_;
        int response_size = static_cast<int>(response_data.length());
        return ProcessingResult{response_data, response_size};
    }
    
    std::string get_type() const override { return "Network"; }
    int get_priority() const override { return 6; }
    
    static NetworkTask from_config(const std::string& config) {
        std::string url = "http://example.com";
        int timeout = 1000;
        
        if (config.find("url=") != std::string::npos) {
            size_t start = config.find("url=") + 4;
            size_t end = config.find(';', start);
            url = config.substr(start, end - start);
        }
        if (config.find("timeout=") != std::string::npos) {
            size_t start = config.find("timeout=") + 8;
            size_t end = config.find(';', start);
            if (end == std::string::npos) end = config.length();
            timeout = std::stoi(config.substr(start, end - start));
        }
        return NetworkTask{url, timeout};
    }
    
private:
    std::string url_;
    int timeout_ms_;
};

// ============================================================================
// SIMPLIFIED SUPPORTING CLASSES
// ============================================================================

enum class ExecutionStrategy {
    Sequential,
    Parallel,
    Pipeline,
    Adaptive
};

class ExecutionEngine {
public:
    void set_execution_strategy(ExecutionStrategy strategy) { strategy_ = strategy; }
    
    std::vector<std::string> execute(const std::vector<std::unique_ptr<TaskBase>>& tasks) {
        std::vector<std::string> results;
        switch (strategy_) {
            case ExecutionStrategy::Sequential:
                return execute_sequential(tasks);
            case ExecutionStrategy::Parallel:
                return execute_parallel(tasks);
            default:
                return execute_sequential(tasks);
        }
    }

private:
    ExecutionStrategy strategy_ = ExecutionStrategy::Sequential;
    
    std::vector<std::string> execute_sequential(const std::vector<std::unique_ptr<TaskBase>>& tasks) {
        std::vector<std::string> results;
        for (const auto& task : tasks) {
            try {
                results.push_back(task->execute());
            } catch (const std::exception& e) {
                results.push_back("Error: " + std::string(e.what()));
            }
        }
        return results;
    }
    
    std::vector<std::string> execute_parallel(const std::vector<std::unique_ptr<TaskBase>>& tasks) {
        std::vector<std::future<std::string>> futures;
        for (const auto& task : tasks) {
            futures.push_back(std::async(std::launch::async, [&task]() {
                try {
                    return task->execute();
                } catch (const std::exception& e) {
                    return std::string("Error: ") + e.what();
                }
            }));
        }
        std::vector<std::string> results;
        for (auto& future : futures) {
            results.push_back(future.get());
        }
        return results;
    }
};

class TaskFactory {
public:
    using TaskCreator = std::function<std::unique_ptr<TaskBase>(const std::string&)>;
    
    template<typename T>
    void register_task(const std::string& type_name) {
        creators_[type_name] = [](const std::string& config) -> std::unique_ptr<TaskBase> {
            return std::make_unique<T>(T::from_config(config));
        };
    }
    
    std::unique_ptr<TaskBase> create_task(const std::string& type, const std::string& config) {
        auto it = creators_.find(type);
        if (it != creators_.end()) {
            return it->second(config);
        }
        throw std::runtime_error("Unknown task type: " + type);
    }
    
    static TaskFactory& instance() {
        static TaskFactory factory;
        return factory;
    }
    
private:
    std::map<std::string, TaskCreator> creators_;
};

// ============================================================================
// MAIN PROCESSOR
// ============================================================================

class DistributedTaskProcessor {
public:
    DistributedTaskProcessor() {
        TaskFactory::instance().register_task<DataProcessingTask>("DataProcessing");
        TaskFactory::instance().register_task<NetworkTask>("Network");
        TaskFactory::instance().register_task<ComputationTask>("Computation");
        execution_engine_.set_execution_strategy(ExecutionStrategy::Parallel);
    }
    
    void set_execution_strategy(ExecutionStrategy strategy) {
        execution_engine_.set_execution_strategy(strategy);
    }
    
    std::vector<std::string> execute_all_tasks() {
        if (pending_tasks_.empty()) {
            std::cout << "No tasks to execute.\n";
            return {};
        }
        
        std::cout << "Executing " << pending_tasks_.size() << " tasks...\n";
        auto start_time = std::chrono::high_resolution_clock::now();
        
        auto results = execution_engine_.execute(pending_tasks_);
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        std::cout << "Execution completed in " << duration.count() << "ms\n";
        return results;
    }
    
    template<typename TaskType, typename... Args>
    void create_and_add_task(Args&&... args) {
        auto task = std::make_unique<TaskType>(std::forward<Args>(args)...);
        pending_tasks_.push_back(std::move(task));
    }
    
    void clear_tasks() {
        pending_tasks_.clear();
    }
    
    void print_task_summary() const {
        std::cout << "\n=== Task Summary ===\n";
        std::cout << "Total tasks: " << pending_tasks_.size() << "\n";
        
        std::map<std::string, int> task_type_counts;
        for (const auto& task : pending_tasks_) {
            task_type_counts[task->get_type()]++;
        }
        
        std::cout << "Task types:\n";
        for (const auto& [type, count] : task_type_counts) {
            std::cout << "  " << type << ": " << count << "\n";
        }
        std::cout << "==================\n\n";
    }

private:
    ExecutionEngine execution_engine_;
    std::vector<std::unique_ptr<TaskBase>> pending_tasks_;
};

} // namespace dtpf

// ============================================================================
// MAIN FUNCTION
// ============================================================================

int main() {
    using namespace dtpf;
    
    std::cout << "=== Distributed Task Processing Framework Demo ===\n\n";
    
    try {
        DistributedTaskProcessor processor;
        
        std::cout << "1. Manual Task Creation Example:\n";
        processor.create_and_add_task<DataProcessingTask>("sample_data_1", 2, 8);
        processor.create_and_add_task<NetworkTask>("https://api.example.com", 500);
        processor.create_and_add_task<ComputationTask>(15, "fibonacci");
        processor.create_and_add_task<DataProcessingTask>("sample_data_2", 1, 5);
        
        processor.print_task_summary();
        
        std::cout << "Executing with PARALLEL strategy:\n";
        processor.set_execution_strategy(ExecutionStrategy::Parallel);
        auto results = processor.execute_all_tasks();
        
        std::cout << "\nResults:\n";
        for (size_t i = 0; i < results.size(); ++i) {
            std::cout << "Task " << i + 1 << ": " << results[i] << "\n";
        }
        
        processor.clear_tasks();
        
        std::cout << "\n2. Strategy Comparison Example:\n";
        processor.create_and_add_task<DataProcessingTask>("dataset_1", 1, 7);
        processor.create_and_add_task<ComputationTask>(10, "factorial");
        processor.create_and_add_task<NetworkTask>("https://service.example.com", 300);
        
        std::vector<ExecutionStrategy> strategies = {
            ExecutionStrategy::Sequential,
            ExecutionStrategy::Parallel
        };
        
        std::vector<std::string> strategy_names = {"SEQUENTIAL", "PARALLEL"};
        
        for (size_t i = 0; i < strategies.size(); ++i) {
            std::cout << "\nTesting " << strategy_names[i] << " strategy:\n";
            processor.set_execution_strategy(strategies[i]);
            
            auto start = std::chrono::high_resolution_clock::now();
            auto test_results = processor.execute_all_tasks();
            auto end = std::chrono::high_resolution_clock::now();
            
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            std::cout << "Strategy: " << strategy_names[i] 
                      << " - Duration: " << duration.count() << "ms\n";
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\n=== Demo completed successfully! ===\n";
    return 0;
}