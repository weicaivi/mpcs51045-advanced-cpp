// concurrency.cpp
// Thread pools, synchronization, and concurrent task processing

#include <thread>
#include <future>
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <vector>
#include <memory>
#include <chrono>
#include <exception>
#include <stack>

namespace dtpf {

// ============================================================================
// THREAD POOL IMPLEMENTATION
// ============================================================================

class ThreadPool {
public:
    explicit ThreadPool(size_t num_threads = std::thread::hardware_concurrency()) 
        : stop_(false) {
        
        if (num_threads == 0) {
            num_threads = 1; // Fallback to at least one thread
        }
        
        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this] { worker_loop(); });
        }
    }
    
    ~ThreadPool() {
        shutdown();
    }
    
    // Non-copyable, non-movable
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;
    
    template<typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
        using return_type = std::invoke_result_t<F, Args...>;
        
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
        auto result = task->get_future();
        
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            if (stop_) {
                throw std::runtime_error("ThreadPool is stopped");
            }
            tasks_.emplace([task] { (*task)(); });
        }
        
        condition_.notify_one();
        return result;
    }
    
    template<typename F, typename... Args>
    auto enqueue_with_priority(int priority, F&& f, Args&&... args) 
        -> std::future<std::invoke_result_t<F, Args...>> {
        
        using return_type = std::invoke_result_t<F, Args...>;
        
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
        auto result = task->get_future();
        
        {
            std::lock_guard<std::mutex> lock(priority_queue_mutex_);
            if (stop_) {
                throw std::runtime_error("ThreadPool is stopped");
            }
            priority_tasks_.emplace(priority, [task] { (*task)(); });
        }
        
        priority_condition_.notify_one();
        return result;
    }
    
    void shutdown() {
        {
            std::lock_guard<std::mutex> lock1(queue_mutex_);
            std::lock_guard<std::mutex> lock2(priority_queue_mutex_);
            stop_ = true;
        }
        
        condition_.notify_all();
        priority_condition_.notify_all();
        
        for (auto& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }
    
    size_t active_threads() const {
        return active_count_.load();
    }
    
    size_t queue_size() const {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        return tasks_.size();
    }
    
    size_t priority_queue_size() const {
        std::lock_guard<std::mutex> lock(priority_queue_mutex_);
        return priority_tasks_.size();
    }

private:
    struct PriorityTask {
        int priority;
        std::function<void()> task;
        
        PriorityTask(int p, std::function<void()> t) : priority(p), task(std::move(t)) {}
        
        bool operator<(const PriorityTask& other) const {
            return priority < other.priority; // Higher priority = larger number
        }
    };
    
    void worker_loop() {
        while (true) {
            std::function<void()> task;
            
            // Try to get priority task first
            {
                std::unique_lock<std::mutex> lock(priority_queue_mutex_);
                if (!priority_tasks_.empty()) {
                    task = priority_tasks_.top().task;
                    priority_tasks_.pop();
                    lock.unlock();
                    
                    execute_task(task);
                    continue;
                }
            }
            
            // Then try regular queue
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                condition_.wait(lock, [this] { 
                    return stop_ || !tasks_.empty() || has_priority_tasks(); 
                });
                
                if (stop_ && tasks_.empty() && !has_priority_tasks()) {
                    return;
                }
                
                if (!tasks_.empty()) {
                    task = std::move(tasks_.front());
                    tasks_.pop();
                    lock.unlock();
                    
                    execute_task(task);
                    continue;
                }
            }
            
            // Check priority queue again before waiting
            if (has_priority_tasks()) {
                continue;
            }
        }
    }
    
    void execute_task(std::function<void()>& task) {
        if (task) {
            active_count_++;
            try {
                task();
            } catch (...) {
                // Log exception in real implementation
            }
            active_count_--;
        }
    }
    
    bool has_priority_tasks() const {
        std::lock_guard<std::mutex> lock(priority_queue_mutex_);
        return !priority_tasks_.empty();
    }
    
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::priority_queue<PriorityTask> priority_tasks_;
    
    mutable std::mutex queue_mutex_;
    mutable std::mutex priority_queue_mutex_;
    std::condition_variable condition_;
    std::condition_variable priority_condition_;
    
    std::atomic<bool> stop_;
    std::atomic<size_t> active_count_{0};
};

// ============================================================================
// WORK-STEALING THREAD POOL
// ============================================================================

class WorkStealingThreadPool {
public:
    explicit WorkStealingThreadPool(size_t num_threads = std::thread::hardware_concurrency())
        : stop_(false), queues_(num_threads), index_(0) {
        
        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this, i] { worker_loop(i); });
        }
    }
    
    ~WorkStealingThreadPool() {
        stop_ = true;
        for (auto& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }
    
    template<typename F, typename... Args>
    auto submit(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
        using return_type = std::invoke_result_t<F, Args...>;
        
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
        auto result = task->get_future();
        
        // Round-robin assignment to worker queues
        size_t queue_index = index_++ % queues_.size();
        queues_[queue_index].push([task] { (*task)(); });
        
        return result;
    }

private:
    class WorkStealingQueue {
    public:
        void push(std::function<void()> task) {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push_back(std::move(task));
        }
        
        bool try_pop(std::function<void()>& task) {
            std::lock_guard<std::mutex> lock(mutex_);
            if (queue_.empty()) {
                return false;
            }
            task = std::move(queue_.front());
            queue_.pop_front();
            return true;
        }
        
        bool try_steal(std::function<void()>& task) {
            std::lock_guard<std::mutex> lock(mutex_);
            if (queue_.empty()) {
                return false;
            }
            task = std::move(queue_.back());
            queue_.pop_back();
            return true;
        }
        
    private:
        mutable std::mutex mutex_;
        std::deque<std::function<void()>> queue_;
    };
    
    void worker_loop(size_t worker_id) {
        while (!stop_) {
            std::function<void()> task;
            
            // Try to get task from own queue first
            if (queues_[worker_id].try_pop(task)) {
                task();
                continue;
            }
            
            // Try to steal from other queues
            for (size_t i = 0; i < queues_.size(); ++i) {
                if (i != worker_id && queues_[i].try_steal(task)) {
                    task();
                    break;
                }
            }
            
            // Brief pause to avoid busy waiting
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
    }
    
    std::vector<std::thread> workers_;
    std::vector<WorkStealingQueue> queues_;
    std::atomic<bool> stop_;
    std::atomic<size_t> index_;
};

// ============================================================================
// SYNCHRONIZATION UTILITIES
// ============================================================================

class Barrier {
public:
    explicit Barrier(size_t count) : count_(count), waiting_(0), generation_(0) {}
    
    void wait() {
        std::unique_lock<std::mutex> lock(mutex_);
        size_t gen = generation_;
        
        if (++waiting_ == count_) {
            generation_++;
            waiting_ = 0;
            condition_.notify_all();
        } else {
            condition_.wait(lock, [this, gen] { return gen != generation_; });
        }
    }
    
private:
    size_t count_;
    size_t waiting_;
    size_t generation_;
    std::mutex mutex_;
    std::condition_variable condition_;
};

class CountDownLatch {
public:
    explicit CountDownLatch(size_t count) : count_(count) {}
    
    void count_down() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (count_ > 0) {
            --count_;
            if (count_ == 0) {
                condition_.notify_all();
            }
        }
    }
    
    void wait() {
        std::unique_lock<std::mutex> lock(mutex_);
        condition_.wait(lock, [this] { return count_ == 0; });
    }
    
    template<typename Rep, typename Period>
    bool wait_for(const std::chrono::duration<Rep, Period>& timeout) {
        std::unique_lock<std::mutex> lock(mutex_);
        return condition_.wait_for(lock, timeout, [this] { return count_ == 0; });
    }
    
private:
    size_t count_;
    std::mutex mutex_;
    std::condition_variable condition_;
};

// ============================================================================
// CONCURRENT TASK SCHEDULER
// ============================================================================

class TaskScheduler {
public:
    enum class SchedulingPolicy {
        RoundRobin,
        Priority,
        LoadBased,
        WorkStealing
    };
    
    explicit TaskScheduler(SchedulingPolicy policy = SchedulingPolicy::Priority, 
                          size_t num_threads = std::thread::hardware_concurrency())
        : policy_(policy) {
        
        switch (policy) {
            case SchedulingPolicy::WorkStealing:
                work_stealing_pool_ = std::make_unique<WorkStealingThreadPool>(num_threads);
                break;
            default:
                thread_pool_ = std::make_unique<ThreadPool>(num_threads);
                break;
        }
    }
    
    template<typename F, typename... Args>
    auto schedule_task(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
        switch (policy_) {
            case SchedulingPolicy::WorkStealing:
                return work_stealing_pool_->submit(std::forward<F>(f), std::forward<Args>(args)...);
            default:
                return thread_pool_->enqueue(std::forward<F>(f), std::forward<Args>(args)...);
        }
    }
    
    template<typename F, typename... Args>
    auto schedule_priority_task(int priority, F&& f, Args&&... args) 
        -> std::future<std::invoke_result_t<F, Args...>> {
        
        if (policy_ == SchedulingPolicy::Priority && thread_pool_) {
            return thread_pool_->enqueue_with_priority(priority, 
                std::forward<F>(f), std::forward<Args>(args)...);
        } else {
            return schedule_task(std::forward<F>(f), std::forward<Args>(args)...);
        }
    }
    
    void shutdown() {
        if (thread_pool_) {
            thread_pool_->shutdown();
        }
        // work_stealing_pool_ shuts down in destructor
    }
    
private:
    SchedulingPolicy policy_;
    std::unique_ptr<ThreadPool> thread_pool_;
    std::unique_ptr<WorkStealingThreadPool> work_stealing_pool_;
};

// ============================================================================
// CONCURRENT DATA STRUCTURES
// ============================================================================

template<typename T>
class ConcurrentQueue {
public:
    void push(T item) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(std::move(item));
        condition_.notify_one();
    }
    
    bool try_pop(T& item) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) {
            return false;
        }
        item = std::move(queue_.front());
        queue_.pop();
        return true;
    }
    
    void wait_and_pop(T& item) {
        std::unique_lock<std::mutex> lock(mutex_);
        condition_.wait(lock, [this] { return !queue_.empty(); });
        item = std::move(queue_.front());
        queue_.pop();
    }
    
    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }
    
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }
    
private:
    mutable std::mutex mutex_;
    std::queue<T> queue_;
    std::condition_variable condition_;
};

template<typename T>
class ConcurrentStack {
public:
    void push(T item) {
        std::lock_guard<std::mutex> lock(mutex_);
        stack_.push(std::move(item));
    }
    
    bool try_pop(T& item) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (stack_.empty()) {
            return false;
        }
        item = std::move(stack_.top());
        stack_.pop();
        return true;
    }
    
    std::shared_ptr<T> try_pop() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (stack_.empty()) {
            return std::shared_ptr<T>();
        }
        auto result = std::make_shared<T>(std::move(stack_.top()));
        stack_.pop();
        return result;
    }
    
    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return stack_.empty();
    }
    
private:
    mutable std::mutex mutex_;
    std::stack<T> stack_;
};

// ============================================================================
// ASYNC RESULT AGGREGATOR
// ============================================================================

template<typename T>
class AsyncResultAggregator {
public:
    void add_future(std::future<T> future) {
        futures_.push_back(std::move(future));
    }
    
    std::vector<T> wait_for_all() {
        std::vector<T> results;
        results.reserve(futures_.size());
        
        for (auto& future : futures_) {
            results.push_back(future.get());
        }
        
        return results;
    }
    
    std::vector<T> wait_for_all_with_timeout(std::chrono::milliseconds timeout) {
        std::vector<T> results;
        results.reserve(futures_.size());
        
        for (auto& future : futures_) {
            if (future.wait_for(timeout) == std::future_status::ready) {
                results.push_back(future.get());
            } else {
                throw std::runtime_error("Future timed out");
            }
        }
        
        return results;
    }
    
    size_t count() const {
        return futures_.size();
    }
    
private:
    std::vector<std::future<T>> futures_;
};

} // namespace dtpf