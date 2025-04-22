#include <iostream>
#include <type_traits>

template<class T>
struct my_is_reference {
    static constexpr bool value = false;
};

// Specialization for lvalue references
template<class T>
struct my_is_reference<T&> {
    static constexpr bool value = true;
};

// Specialization for rvalue references
template<class T>
struct my_is_reference<T&&> {
    static constexpr bool value = true;
};

// Helper variable template
template<class T>
inline constexpr bool my_is_reference_v = my_is_reference<T>::value;


template<class T>
struct my_remove_reference {
    using type = T;
};

// Specialization for lvalue references
template<class T>
struct my_remove_reference<T&> {
    using type = T;
};

// Specialization for rvalue references
template<class T>
struct my_remove_reference<T&&> {
    using type = T;
};

// Helper alias template
template<class T>
using my_remove_reference_t = typename my_remove_reference<T>::type;

int main() {
    // Test is_reference trait
    std::cout << "Testing my_is_reference trait:" << std::endl;
    std::cout << "my_is_reference<int>::value = " << my_is_reference<int>::value << std::endl;
    std::cout << "my_is_reference<int&>::value = " << my_is_reference<int&>::value << std::endl;
    std::cout << "my_is_reference<int&&>::value = " << my_is_reference<int&&>::value << std::endl;
    std::cout << "my_is_reference_v<int> = " << my_is_reference_v<int> << std::endl;
    std::cout << "my_is_reference_v<int&> = " << my_is_reference_v<int&> << std::endl;
    std::cout << "my_is_reference_v<int&&> = " << my_is_reference_v<int&&> << std::endl;
    
    // Verify against standard implementation
    std::cout << "\nComparing with std::is_reference:" << std::endl;
    std::cout << "std::is_reference<int>::value = " << std::is_reference<int>::value << std::endl;
    std::cout << "std::is_reference<int&>::value = " << std::is_reference<int&>::value << std::endl;
    std::cout << "std::is_reference<int&&>::value = " << std::is_reference<int&&>::value << std::endl;
    
    // Test remove_reference trait
    std::cout << "\nTesting my_remove_reference trait:" << std::endl;
    
    bool is_same_int = std::is_same<int, my_remove_reference_t<int>>::value;
    bool is_same_int_lref = std::is_same<int, my_remove_reference_t<int&>>::value;
    bool is_same_int_rref = std::is_same<int, my_remove_reference_t<int&&>>::value;
    
    std::cout << "std::is_same<int, my_remove_reference_t<int>>::value = " << is_same_int << std::endl;
    std::cout << "std::is_same<int, my_remove_reference_t<int&>>::value = " << is_same_int_lref << std::endl;
    std::cout << "std::is_same<int, my_remove_reference_t<int&&>>::value = " << is_same_int_rref << std::endl;
    
    // Verify against standard implementation
    std::cout << "\nComparing with std::remove_reference:" << std::endl;
    
    bool std_is_same_int = std::is_same<int, std::remove_reference_t<int>>::value;
    bool std_is_same_int_lref = std::is_same<int, std::remove_reference_t<int&>>::value;
    bool std_is_same_int_rref = std::is_same<int, std::remove_reference_t<int&&>>::value;
    
    std::cout << "std::is_same<int, std::remove_reference_t<int>>::value = " << std_is_same_int << std::endl;
    std::cout << "std::is_same<int, std::remove_reference_t<int&>>::value = " << std_is_same_int_lref << std::endl;
    std::cout << "std::is_same<int, std::remove_reference_t<int&&>>::value = " << std_is_same_int_rref << std::endl;
    
    // Add static assertions to verify correctness at compile time
    static_assert(my_is_reference<int&>::value, "my_is_reference<int&> should be true");
    static_assert(my_is_reference<int&&>::value, "my_is_reference<int&&> should be true");
    static_assert(!my_is_reference<int>::value, "my_is_reference<int> should be false");
    
    static_assert(std::is_same<int, my_remove_reference_t<int>>::value, "my_remove_reference_t<int> should be int");
    static_assert(std::is_same<int, my_remove_reference_t<int&>>::value, "my_remove_reference_t<int&> should be int");
    static_assert(std::is_same<int, my_remove_reference_t<int&&>>::value, "my_remove_reference_t<int&&> should be int");
    
    return 0;
}