#ifndef MY_PROMISE_H
#define MY_PROMISE_H

#include <optional>
#include <variant>
#include <memory>
#include <exception>
#include <stdexcept>
#include <atomic>
#include <functional>

namespace mpcs {

// Helper template for std::visit with lambdas
template<class... Ts> 
struct overloaded : Ts... { 
    using Ts::operator()...; 
};
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

template<class T> class MyPromise;

// Using variant to hold either a value or an exception
template<class T>
struct SharedState {
    // use std::variant to store either a value of type T or an exception_ptr
    using ValueVariant = std::variant<std::monostate, T, std::exception_ptr>;
    
    std::atomic<bool> ready{false};
    std::atomic<bool> consumer_waiting{false};
    
    ValueVariant value;
    
    // Atomic notify/wait mechanisms
    std::atomic_flag notifier = ATOMIC_FLAG_INIT;
    
    void notify() {
        // First ensure all writes to value are visible
        // Use release to ensure all prior writes are visible to other threads
        ready.store(true, std::memory_order_release);
        
        // Use relaxed for the test_and_set since we only care about the notify
        // The synchronization is already handled by ready.store with release semantics
        notifier.test_and_set(std::memory_order_relaxed);
        notifier.notify_one();
    }
    
    void wait() {
        // Can use relaxed here as we're just recording state, not synchronizing
        consumer_waiting.store(true, std::memory_order_relaxed);
        
        // Use acquire to ensure we see all writes made before the ready flag was set
        while (!ready.load(std::memory_order_acquire)) {
            // Use acquire to ensure we see all memory operations that happened before the notify
            notifier.wait(false, std::memory_order_acquire);
        }
    }
};

template<typename T>
class MyFuture {
public:
    MyFuture(const MyFuture&) = delete;
    MyFuture(MyFuture&&) = default;
    
    T get() {
        sharedState->wait();
        
        return std::visit(overloaded {
            [](std::monostate&) -> T {
                throw std::runtime_error{"Future accessed but no value set"};
            },
            [](T& value) -> T {
                return std::move(value);
            },
            [](std::exception_ptr& exc) -> T {
                std::rethrow_exception(exc);
            }
        }, sharedState->value);
    }
    
    bool is_ready() const {
        // Use acquire to ensure we see any updates to the value if ready is true
        return sharedState->ready.load(std::memory_order_acquire);
    }

private:
    friend class MyPromise<T>;
    explicit MyFuture(std::shared_ptr<SharedState<T>>& state) : sharedState(state) {}
    std::shared_ptr<SharedState<T>> sharedState;
};

template<typename T>
class MyPromise {
public:
    MyPromise() : sharedState{std::make_shared<SharedState<T>>()} {}
    
    void set_value(T value) {
        // Can use relaxed here as we're just checking, not synchronizing
        if (sharedState->ready.load(std::memory_order_relaxed)) {
            throw std::runtime_error{"Promise value already set"};
        }
        
        sharedState->value = std::move(value);
        sharedState->notify();
    }
    
    void set_exception(std::exception_ptr exc) {
        // Can use relaxed here as we're just checking, not synchronizing
        if (sharedState->ready.load(std::memory_order_relaxed)) {
            throw std::runtime_error{"Promise value already set"};
        }
        
        sharedState->value = exc;
        sharedState->notify();
    }
    
    MyFuture<T> get_future() {
        return MyFuture<T>{sharedState};
    }
    
    bool has_consumer() const {
        // just reading state, not synchronizing
        return sharedState->consumer_waiting.load(std::memory_order_relaxed);
    }

private:
    std::shared_ptr<SharedState<T>> sharedState;
};

}

#endif