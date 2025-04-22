#include <iostream>
#include <vector>
#include <iterator>
#include <algorithm>
#include <string>
#include <string_view>

// ostream_joiner class template
template<typename T, typename CharT = char, typename Traits = std::char_traits<CharT>>
class ostream_joiner {
public:
    // Iterator traits
    using iterator_category = std::output_iterator_tag;
    using value_type = void;
    using difference_type = void;
    using pointer = void;
    using reference = void;
    using char_type = CharT;
    using traits_type = Traits;
    using ostream_type = std::basic_ostream<CharT, Traits>;

    // Constructor taking an output stream and a delimiter
    ostream_joiner(ostream_type& os, const CharT* delim) 
        : os_(&os), delimiter_(delim), first_elem_(true) {}
    
    // Constructor taking an output stream and any delimiter type that can be streamed
    template<typename DelimT>
    ostream_joiner(ostream_type& os, const DelimT& delim)
        : os_(&os), delimiter_(delim), first_elem_(true) {}

    // operator* returns a reference to self, allowing *oi = value to work
    ostream_joiner& operator*() { 
        return *this; 
    }
    
    // Assignment operator - called when *oi = value is executed
    template<typename U>
    ostream_joiner& operator=(const U& value) {
        // If this is not the first element, output the delimiter first
        if (!first_elem_) {
            *os_ << delimiter_;
        } else {
            first_elem_ = false;
        }
        
        *os_ << value;
        return *this;
    }

    // Required for output iterators
    ostream_joiner& operator++() { return *this; }
    ostream_joiner& operator++(int) { return *this; }

private:
    ostream_type* os_;       // Pointer to the output stream
    std::basic_string<CharT> delimiter_;  // Delimiter between elements
    bool first_elem_;        // Flag to track if this is the first element
};

// Deduction guides for C++17 Class Template Argument Deduction (CTAD)
template<typename CharT, typename Traits>
ostream_joiner(std::basic_ostream<CharT, Traits>&, const CharT*) 
    -> ostream_joiner<void, CharT, Traits>;

template<typename CharT, typename Traits, typename DelimT>
ostream_joiner(std::basic_ostream<CharT, Traits>&, const DelimT&) 
    -> ostream_joiner<void, CharT, Traits>;

// Helper function to create an ostream_joiner (pre-C++17 alternative to CTAD)
template<typename CharT, typename Traits, typename DelimT>
ostream_joiner<void, CharT, Traits> make_ostream_joiner(
    std::basic_ostream<CharT, Traits>& os, const DelimT& delimiter) {
    return ostream_joiner<void, CharT, Traits>(os, delimiter);
}

// Generic operator<< for any vector type
template<typename T, typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& operator<<(
    std::basic_ostream<CharT, Traits>& os, const std::vector<T>& vec) {
    
    os << CharT('[');
    std::copy(vec.begin(), vec.end(), 
              ostream_joiner<void, CharT, Traits>(os, CharT(',') + std::basic_string<CharT>(1, CharT(' '))));
    return os << CharT(']');
}

int main() {
    std::vector<int> v = {1, 2, 3, 4, 5};
    
    std::cout << "Using ostream_iterator: ";
    std::copy(v.begin(), v.end(), std::ostream_iterator<int>(std::cout, ", "));
    std::cout << std::endl;
    
    std::cout << "Using ostream_joiner: ";
    std::copy(v.begin(), v.end(), ostream_joiner(std::cout, ", "));
    std::cout << std::endl;
    
    std::cout << "Using vector operator<<: " << v << std::endl;
    
    std::vector<double> empty_vec;
    std::cout << "Empty vector: " << empty_vec << std::endl;
    
    std::vector<std::string> str_vec = {"matcha", "hojicha", "black tea"};
    std::cout << "String vector: " << str_vec << std::endl;
    
    std::vector<int> wv = {6, 7, 8, 9, 10};
    std::wcout << L"Wide char output: ";
    std::copy(wv.begin(), wv.end(), ostream_joiner(std::wcout, L", "));
    std::wcout << std::endl;
    
    std::wcout << L"Wide char vector operator<<: " << wv << std::endl;
    
    return 0;
}