// Concepts, type traits, and template metaprogramming utilities

#include <concepts>
#include <string>
#include <type_traits>
#include <functional>
#include <tuple>

namespace dtpf {

// ============================================================================
// CONCEPTS: Type constraints for compile-time validation
// ============================================================================

template<typename T>
concept Serializable = requires(T t) {
    { t.serialize() } -> std::convertible_to<std::string>;
    { T::deserialize(std::string{}) } -> std::same_as<T>;
};

template<typename T>
concept TaskResult = std::movable<T> && Serializable<T>;

template<typename F, typename... Args>
concept TaskFunction = std::invocable<F, Args...> && 
                      TaskResult<std::invoke_result_t<F, Args...>>;

// ============================================================================
// TEMPLATE METAPROGRAMMING: Compile-time pipeline validation
// ============================================================================

template<typename... Tasks>
struct TaskPipeline {
    static constexpr size_t task_count = sizeof...(Tasks);
    
    template<size_t N>
    using task_type = std::tuple_element_t<N, std::tuple<Tasks...>>;
    
    // Validate pipeline compatibility at compile time
    static constexpr bool is_valid_pipeline() {
        if constexpr (task_count <= 1) {
            return true;
        } else {
            return validate_chain<0>();
        }
    }
    
private:
    template<size_t I>
    static constexpr bool validate_chain() {
        if constexpr (I >= task_count - 1) {
            return true;
        } else {
            using current = task_type<I>;
            using next = task_type<I + 1>;
            
            // Validate task compatibility
            static_assert(std::is_class_v<current>, "Task must be a class type");
            static_assert(std::is_class_v<next>, "Task must be a class type");
            
            return validate_chain<I + 1>();
        }
    }
};

// ============================================================================
// TYPE TRAITS: Helper utilities for template metaprogramming
// ============================================================================

template<typename T>
struct has_execute {
private:
    template<typename U>
    static auto test(int) -> decltype(std::declval<U>().execute(), std::true_type{});
    
    template<typename>
    static std::false_type test(...);
    
public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

template<typename T>
constexpr bool has_execute_v = has_execute<T>::value;

template<typename T>
struct has_priority {
private:
    template<typename U>
    static auto test(int) -> decltype(std::declval<U>().get_priority(), std::true_type{});
    
    template<typename>
    static std::false_type test(...);
    
public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

template<typename T>
constexpr bool has_priority_v = has_priority<T>::value;

// Template parameter pack utilities
template<template<typename> class Predicate, typename... Types>
constexpr size_t count_if_v = (Predicate<Types>::value + ...);

template<template<typename> class Predicate, typename... Types>
constexpr bool all_of_v = (Predicate<Types>::value && ...);

template<template<typename> class Predicate, typename... Types>
constexpr bool any_of_v = (Predicate<Types>::value || ...);

}