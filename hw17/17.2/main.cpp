#include <iostream>
#include <type_traits>
#include <tuple>

using namespace std;

// Helper for detecting if a type satisfies a trait
template<typename T, template<typename> typename Trait, typename = void>
struct satisfies_trait : false_type {};

template<typename T, template<typename> typename Trait>
struct satisfies_trait<T, Trait, void_t<typename enable_if<Trait<T>::value>::type>> : true_type {};

template<typename T, template<typename> typename Trait>
concept SatisfiesTrait = satisfies_trait<T, Trait>::value;

// Primary template for filtering - empty case
template<typename Tuple, template<typename> typename Trait, typename Result = tuple<>>
struct Filter {
    using type = Result;
};

// Specialization for non-empty tuple
template<typename Head, typename... Tail, template<typename> typename Trait, typename... Result>
struct Filter<tuple<Head, Tail...>, Trait, tuple<Result...>> {
    using type = typename conditional
        SatisfiesTrait<Head, Trait>,
        typename Filter<tuple<Tail...>, Trait, tuple<Result..., Head>>::type,
        typename Filter<tuple<Tail...>, Trait, tuple<Result...>>::type
    >::type;
};

// Type alias for easier use
template<typename Tuple, template<typename> typename Trait>
using Filter_t = typename Filter<Tuple, Trait>::type;

// Helper for printing tuple types
template<typename T>
struct print_type;

template<typename... Ts>
struct print_type<tuple<Ts...>> {
    static void print() {
        std::cout << "tuple<";
        int i = 0;
        ((std::cout << (i++ ? ", " : "") << typeid(Ts).name()), ...);
        std::cout << ">" << std::endl;
    }
};

int main() {
    // Define test types
    using TestTuple = tuple<int, float, long, double, char, bool>;
    
    // Test with is_integral
    using IntegralTypes = Filter_t<TestTuple, is_integral>;
    std::cout << "Original tuple: ";
    print_type<TestTuple>::print();
    
    std::cout << "Filtered integral types: ";
    print_type<IntegralTypes>::print();
    
    // Compile-time verification with static assertions
    static_assert(is_same_v<Filter_t<tuple<long, float>, is_integral>, tuple<long>>);
    static_assert(is_same_v<Filter_t<tuple<int, float, char>, is_integral>, tuple<int, char>>);
    static_assert(is_same_v<Filter_t<tuple<float, double>, is_integral>, tuple<>>);
    
    std::cout << "All static assertions passed!" << std::endl;
    
    return 0;
}