/**
Discuss why and where we would use each one of the four different notations for
 using concepts to constrain templates with examples.
*/


// 1. Template Header with requires Clause
template<typename T> 
requires copy_constructible<T> 
T duplicate(T const &t) { return t; }
/**
 * This is best for: 
 * Complex constraints involving multiple concepts and logical operators
 * when we need to express relationships between multiple template parameters
 * When constraints involve non-type template parameters
 */
// example
template<typename T, typename U>
requires std::copyable<T> && std::convertible_to<U, T> && (!std::is_reference_v<T>)
T combine(T a, U b) {
    return a + static_cast<T>(b);
}

// 2. Trailing requires Clause
template<typename T> 
T duplicate(T const &t) requires copy_constructible<T> { return t; }
/**
 * This is best for: 
 * When different functions in a class template have different constraints
 * Function template specializations with distinct constraints
 * When constraints are specific to a particular function, not the whole class
 */
// example
template<typename T>
class Container {
    // Only available if T is copyable
    void copy_all(const Container& other) requires std::copyable<T>;
    
    // Only available if T is movable
    void fast_copy(Container&& other) requires std::movable<T>;
    
    // Always available
    size_t size() const;
};

// 3. Constrained Template Parameter
template<copy_constructible T>
T duplicate(T const &t) {return t;}
/**
 * This is best for: 
 * Simple, single concept constraints
 * When the constraint is fundamental to the parameter's role
 * Cleaner, more concise code when only basic constraints are needed
 * When all uses of the parameter require the same constraint
 */
// example
template<std::forward_iterator It, std::sentinel_for<It> Sent>
void traverse(It first, Sent last) {
    while (first != last) {
        /* process *first */
        ++first;
    }
}

// 4. Abbreviated Function Template with auto
auto duplicate(copy_constructible auto const &t) { return t; }
/**
 * This is best for: 
 * Very simple functions with minimal template syntax
 * When we want concise, expressive code
 * Generic lambdas
 * When the template parameter is used only for the type of one function parameter
 */
// example
auto sort_by = []<std::totally_ordered T>(std::vector<T>& vec, 
                                          std::invocable<T, T> auto&& compare) {
    std::sort(vec.begin(), vec.end(), 
              std::forward<decltype(compare)>(compare));
};