#include <iostream>
#include <type_traits>

template<class T>
struct remove_all_pointers {
    using type = T; // Return transformed type in type
};

// Specialization for pointer types
template<class T>
struct remove_all_pointers<T*> {
    using type = typename remove_all_pointers<T>::type;
};

// Helper alias template
template<class T>
using remove_all_pointers_t = typename remove_all_pointers<T>::type;

template<typename T>
void f(T t) {
    remove_all_pointers_t<T> rt;
    std::cout << "Type after removing all pointers: " << typeid(rt).name() << std::endl;
}

int main() {    
    // Test with int
    std::cout << "remove_all_pointers_t<int> is same as int: " 
              << std::is_same<int, remove_all_pointers_t<int>>::value << std::endl;
    
    // Test with int*
    std::cout << "remove_all_pointers_t<int*> is same as int: " 
              << std::is_same<int, remove_all_pointers_t<int*>>::value << std::endl;
    
    // Test with int**
    std::cout << "remove_all_pointers_t<int**> is same as int: " 
              << std::is_same<int, remove_all_pointers_t<int**>>::value << std::endl;
    
    // Test with int***
    std::cout << "remove_all_pointers_t<int***> is same as int: " 
              << std::is_same<int, remove_all_pointers_t<int***>>::value << std::endl;
    
    std::cout << "\nExample usage with template function f():" << std::endl;
    int value = 42;
    int* ptr = &value;
    int** ptr_to_ptr = &ptr;
    
    f(value);        // T is int
    f(ptr);          // T is int*
    f(ptr_to_ptr);   // T is int**
    
    return 0;
}