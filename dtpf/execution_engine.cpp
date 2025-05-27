// execution_engine.cpp
// Simplified execution engine without std::execution (for compatibility)

#include <memory>
#include <vector>
#include <string>
#include <future>
#include <functional>
#include <exception>
#include <chrono>
#include <thread>
#include <iostream>
#include <algorithm>

namespace dtpf {

// Forward declarations
class TaskBase {
public:
    virtual ~TaskBase() = default;
    virtual std::string execute() = 0;
    virtual std::string get_type() const = 0;
    virtual int get_priority() const = 0;
};

// ============================================================================
// EXECUTION POLICIES AND STRATEGIES
// ============================================================================

enum class ExecutionStrategy {
    Sequential,
    Parallel,
    Pipeline,
    Distributed,
    Adaptive
};

struct ExecutionPolicy {
    ExecutionStrategy strategy = ExecutionStrategy::Parallel;
    size_t max_concurrency = std::thread::hardware_concurrency();
    std::chrono::milliseconds timeout{30000};
    bool retry_on_failure = true;
    int max_retries = 3;
    std::vector<std::string> preferred_nodes;
};

// ============================================================================
// SIMPLE EXECUTION ENGINE
// ============================================================================

class ExecutionEngine {
public:
    ExecutionEngine() = default;
    
    void set_execution_policy(const ExecutionPolicy& policy) {
        policy_ = policy;
    }
    
    void set_execution_strategy(ExecutionStrategy strategy) {
        policy_.strategy = strategy;
    }
    
    void add_preferred_nodes(const std::vector<std::string>& nodes) {
        policy_.preferred_nodes = nodes;
    }
    
    // Execute tasks based on current strategy
    std::vector<std::string> execute(const std::vector<std::unique_ptr<TaskBase>>& tasks) {
        if (tasks.empty()) {
            return {};
        }
        
        switch (policy_.strategy) {
            case ExecutionStrategy::Sequential:
                return execute_sequential(tasks);
            case ExecutionStrategy::Parallel:
                return execute_parallel(tasks);
            case ExecutionStrategy::Pipeline:
                return execute_pipeline(tasks);
            case ExecutionStrategy::Distributed:
                return execute_distributed(tasks);
            case ExecutionStrategy::Adaptive:
                return execute_adaptive(tasks);
            default:
                return execute_parallel(tasks);
        }
    }

private:
    ExecutionPolicy policy_;
    
    // Execute tasks one by one
    std::vector<std::string> execute_sequential(const std::vector<std::unique_ptr<TaskBase>>& tasks) {
        std::vector<std::string> results;
        results.reserve(tasks.size());
        
        std::cout << "  → Sequential execution of " << tasks.size() << " tasks\n";
        
        for (size_t i = 0; i < tasks.size(); ++i) {
            try {
                std::cout << "    Executing task " << (i + 1) << "/" << tasks.size() 
                         << " (" << tasks[i]->get_type() << ")\n";
                results.push_back(tasks[i]->execute());
            } catch (const std::exception& e) {
                results.push_back("Error: " + std::string(e.what()));
            }
        }
        
        return results;
    }
    
    // Execute tasks in parallel using std::async
    std::vector<std::string> execute_parallel(const std::vector<std::unique_ptr<TaskBase>>& tasks) {
        std::cout << "  → Parallel execution of " << tasks.size() << " tasks\n";
        
        std::vector<std::future<std::string>> futures;
        futures.reserve(tasks.size());
        
        // Launch all tasks asynchronously
        for (size_t i = 0; i < tasks.size(); ++i) {
            futures.push_back(
                std::async(std::launch::async, [&tasks, i]() {
                    try {
                        return tasks[i]->execute();
                    } catch (const std::exception& e) {
                        return std::string("Error: ") + e.what();
                    }
                })
            );
        }
        
        // Collect results
        std::vector<std::string> results;
        results.reserve(futures.size());
        
        for (size_t i = 0; i < futures.size(); ++i) {
            std::cout << "    Waiting for task " << (i + 1) << "/" << futures.size() << "\n";
            results.push_back(futures[i].get());
        }
        
        return results;
    }
    
    // Execute tasks as pipeline (output of one feeds into next)
    std::vector<std::string> execute_pipeline(const std::vector<std::unique_ptr<TaskBase>>& tasks) {
        std::cout << "  → Pipeline execution of " << tasks.size() << " tasks\n";
        
        std::vector<std::string> results;
        std::string pipeline_input = "initial_input";
        
        for (size_t i = 0; i < tasks.size(); ++i) {
            try {
                std::cout << "    Pipeline stage " << (i + 1) << "/" << tasks.size() 
                         << " (" << tasks[i]->get_type() << ")\n";
                
                // In a real pipeline, you'd pass pipeline_input to the task
                // For demonstration, we'll just execute each task
                std::string result = tasks[i]->execute();
                results.push_back(result);
                pipeline_input = result; // Use result as input for next task
            } catch (const std::exception& e) {
                std::string error = "Pipeline error at stage " + std::to_string(i + 1) + ": " + e.what();
                results.push_back(error);
                break; // Stop pipeline on error
            }
        }
        
        return results;
    }
    
    // Simulate distributed execution
    std::vector<std::string> execute_distributed(const std::vector<std::unique_ptr<TaskBase>>& tasks) {
        std::cout << "  → Distributed execution of " << tasks.size() << " tasks across nodes\n";
        
        std::vector<std::future<std::string>> distributed_futures;
        distributed_futures.reserve(tasks.size());
        
        for (size_t i = 0; i < tasks.size(); ++i) {
            std::string node_id = policy_.preferred_nodes.empty() 
                ? "node_" + std::to_string(i % 3)  // Round-robin default
                : policy_.preferred_nodes[i % policy_.preferred_nodes.size()];
            
            std::cout << "    Assigning task " << (i + 1) << " to " << node_id << "\n";
            
            // Simulate distributed execution with additional latency
            distributed_futures.push_back(
                std::async(std::launch::async, [&tasks, i, node_id]() {
                    try {
                        // Simulate network latency for distributed execution
                        std::this_thread::sleep_for(std::chrono::milliseconds(50 + (i * 10)));
                        
                        std::string result = tasks[i]->execute();
                        return "[" + node_id + "] " + result;
                    } catch (const std::exception& e) {
                        return "[" + node_id + "] Error: " + std::string(e.what());
                    }
                })
            );
        }
        
        // Collect distributed results
        std::vector<std::string> results;
        results.reserve(distributed_futures.size());
        
        for (size_t i = 0; i < distributed_futures.size(); ++i) {
            std::cout << "    Collecting result from distributed task " << (i + 1) << "\n";
            results.push_back(distributed_futures[i].get());
        }
        
        return results;
    }
    
    // Adaptive execution chooses strategy based on task characteristics
    std::vector<std::string> execute_adaptive(const std::vector<std::unique_ptr<TaskBase>>& tasks) {
        std::cout << "  → Adaptive execution analyzing " << tasks.size() << " tasks...\n";
        
        // Analyze task characteristics
        bool has_high_priority = false;
        bool has_computation_tasks = false;
        size_t total_tasks = tasks.size();
        int avg_priority = 0;
        
        for (const auto& task : tasks) {
            int priority = task->get_priority();
            avg_priority += priority;
            
            if (priority > 7) {
                has_high_priority = true;
            }
            
            if (task->get_type() == "Computation") {
                has_computation_tasks = true;
            }
        }
        
        avg_priority /= static_cast<int>(total_tasks);
        
        // Choose strategy based on analysis
        ExecutionStrategy chosen_strategy;
        
        if (total_tasks == 1) {
            chosen_strategy = ExecutionStrategy::Sequential;
            std::cout << "    Adaptive choice: Sequential (single task)\n";
        } else if (has_computation_tasks && total_tasks > 2) {
            chosen_strategy = ExecutionStrategy::Parallel;
            std::cout << "    Adaptive choice: Parallel (computation-heavy tasks)\n";
        } else if (has_high_priority) {
            chosen_strategy = ExecutionStrategy::Parallel;
            std::cout << "    Adaptive choice: Parallel (high-priority tasks)\n";
        } else if (total_tasks > 5 && !policy_.preferred_nodes.empty()) {
            chosen_strategy = ExecutionStrategy::Distributed;
            std::cout << "    Adaptive choice: Distributed (large task set with nodes)\n";
        } else if (avg_priority < 5) {
            chosen_strategy = ExecutionStrategy::Pipeline;
            std::cout << "    Adaptive choice: Pipeline (low-priority sequential tasks)\n";
        } else {
            chosen_strategy = ExecutionStrategy::Parallel;
            std::cout << "    Adaptive choice: Parallel (default)\n";
        }
        
        // Temporarily change strategy and execute
        ExecutionStrategy original_strategy = policy_.strategy;
        policy_.strategy = chosen_strategy;
        
        std::vector<std::string> results = execute(tasks);
        
        // Restore original strategy
        policy_.strategy = original_strategy;
        
        return results;
    }
};

// ============================================================================
// EXECUTION CONTEXT AND UTILITIES
// ============================================================================

class ExecutionContext {
public:
    ExecutionContext(const std::string& name) : context_name_(name) {
        start_time_ = std::chrono::high_resolution_clock::now();
        std::cout << "Starting execution context: " << context_name_ << "\n";
    }
    
    ~ExecutionContext() {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time_);
        std::cout << "Completed execution context: " << context_name_ 
                  << " in " << duration.count() << "ms\n";
    }
    
private:
    std::string context_name_;
    std::chrono::high_resolution_clock::time_point start_time_;
};

// ============================================================================
// TASK PRIORITIZATION AND SORTING
// ============================================================================

class TaskSorter {
public:
    static void sort_by_priority(std::vector<std::unique_ptr<TaskBase>>& tasks, bool descending = true) {
        std::sort(tasks.begin(), tasks.end(), 
            [descending](const std::unique_ptr<TaskBase>& a, const std::unique_ptr<TaskBase>& b) {
                if (descending) {
                    return a->get_priority() > b->get_priority();
                } else {
                    return a->get_priority() < b->get_priority();
                }
            });
    }
    
    static void sort_by_type(std::vector<std::unique_ptr<TaskBase>>& tasks) {
        std::sort(tasks.begin(), tasks.end(),
            [](const std::unique_ptr<TaskBase>& a, const std::unique_ptr<TaskBase>& b) {
                return a->get_type() < b->get_type();
            });
    }
    
    static std::vector<std::vector<std::unique_ptr<TaskBase>>> group_by_priority(
        std::vector<std::unique_ptr<TaskBase>>& tasks) {
        
        // Sort by priority first
        sort_by_priority(tasks);
        
        std::vector<std::vector<std::unique_ptr<TaskBase>>> groups;
        
        if (tasks.empty()) {
            return groups;
        }
        
        int current_priority = tasks[0]->get_priority();
        std::vector<std::unique_ptr<TaskBase>> current_group;
        
        for (auto& task : tasks) {
            if (task->get_priority() != current_priority) {
                if (!current_group.empty()) {
                    groups.push_back(std::move(current_group));
                    current_group.clear();
                }
                current_priority = task->get_priority();
            }
            current_group.push_back(std::move(task));
        }
        
        if (!current_group.empty()) {
            groups.push_back(std::move(current_group));
        }
        
        // Clear original vector since we moved all tasks
        tasks.clear();
        
        return groups;
    }
};

} // namespace dtpf