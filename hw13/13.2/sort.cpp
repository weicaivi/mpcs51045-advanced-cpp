#include <iostream>
#include <list>
#include <vector>
#include <forward_list>
#include <algorithm>
#include <iterator>
#include <chrono>
#include <functional>
#include <type_traits>

// Implementation of sort for forward iterators using merge sort which doesn't require random access
template<typename ForwardIt, typename Compare = std::less<>>
void forward_iterator_sort(ForwardIt first, ForwardIt last, Compare comp = Compare{}) {
    if (first == last || std::next(first) == last) {
        return;
    }

    auto count = std::distance(first, last);
    auto middle = first;
    std::advance(middle, count / 2);

    forward_iterator_sort(first, middle, comp);
    forward_iterator_sort(middle, last, comp);

    // need to create a temporary buffer for forward iterators,
    using value_type = typename std::iterator_traits<ForwardIt>::value_type;
    std::vector<value_type> buffer(count);
    
    auto it1 = first;
    auto it2 = middle;
    auto out = buffer.begin();
    
    while (it1 != middle && it2 != last) {
        if (comp(*it2, *it1)) {
            *out++ = std::move(*it2++);
        } else {
            *out++ = std::move(*it1++);
        }
    }
    
    // Copy remaining elements
    while (it1 != middle) {
        *out++ = std::move(*it1++);
    }
    while (it2 != last) {
        *out++ = std::move(*it2++);
    }
    
    // Copy merged sequence back to the original range
    std::copy(buffer.begin(), buffer.end(), first);
}

// Optimized version for bidirectional iterators
// use in-place merge for bidirectional iterators
template<typename BidirIt, typename Compare = std::less<>>
void bidirectional_iterator_sort(BidirIt first, BidirIt last, Compare comp = Compare{}) {
    // Check if we have bidirectional iterators
    if constexpr (!std::is_base_of_v<std::bidirectional_iterator_tag, 
                                     typename std::iterator_traits<BidirIt>::iterator_category>) {
        // if not fall back to forward_iterator_sort for forward iterators
        forward_iterator_sort(first, last, comp);
        return;
    }
    
    if (first == last || std::next(first) == last) {
        return;
    }

    auto count = std::distance(first, last);
    auto middle = first;
    std::advance(middle, count / 2);

    bidirectional_iterator_sort(first, middle, comp);
    bidirectional_iterator_sort(middle, last, comp);

    // Merge the sorted halves
    std::inplace_merge(first, middle, last, comp);
}

// Unified sort function that dispatches to the appropriate implementation
template<typename Iterator, typename Compare = std::less<>>
void unified_sort(Iterator first, Iterator last, Compare comp = Compare{}) {
    // Dispatch based on iterator category
    if constexpr (std::is_base_of_v<std::random_access_iterator_tag, 
                                    typename std::iterator_traits<Iterator>::iterator_category>) {
        // Use standard sort for random access iterators
        std::sort(first, last, comp);
    } 
    else if constexpr (std::is_base_of_v<std::bidirectional_iterator_tag, 
                                        typename std::iterator_traits<Iterator>::iterator_category>) {
        // Use bidirectional_iterator_sort for bidirectional iterators
        bidirectional_iterator_sort(first, last, comp);
    }
    else {
        // Use forward_iterator_sort for forward iterators
        forward_iterator_sort(first, last, comp);
    }
}

// Helper function to time sorting operations
template<typename Container, typename SortFunc>
double time_sort(Container& container, SortFunc sort_func, const std::string& name) {
    auto start = std::chrono::high_resolution_clock::now();
    sort_func(container.begin(), container.end());
    auto end = std::chrono::high_resolution_clock::now();
    
    std::chrono::duration<double, std::milli> duration = end - start;
    std::cout << name << " took " << duration.count() << " ms" << std::endl;
    
    return duration.count();
}

template<typename T>
void print_container(const T& container, const std::string& label) {
    std::cout << label << ": ";
    for (const auto& item : container) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}

int main() {
    const int SIZE = 10000;
    
    // Test with std::list (bidirectional iterator)
    std::list<int> list_data;
    for (int i = 0; i < SIZE; ++i) {
        list_data.push_front(rand() % 1000);
    }
    
    std::list<int> list_copy = list_data;
    
    std::cout << "Sorting std::list with " << SIZE << " elements:" << std::endl;
    time_sort(list_data, [](auto first, auto last) {
        list_data.sort();
    }, "list::sort()");
    
    time_sort(list_copy, [](auto first, auto last) {
        unified_sort(first, last);
    }, "unified_sort()");
    
    // Test with std::forward_list (forward iterator)
    std::forward_list<int> fwd_list_data;
    for (int i = 0; i < SIZE; ++i) {
        fwd_list_data.push_front(rand() % 1000);
    }
    
    std::forward_list<int> fwd_list_copy = fwd_list_data;
    
    std::cout << "\nSorting std::forward_list with " << SIZE << " elements:" << std::endl;
    time_sort(fwd_list_data, [](auto first, auto last) {
        fwd_list_data.sort();
    }, "forward_list::sort()");
    
    time_sort(fwd_list_copy, [](auto first, auto last) {
        unified_sort(first, last);
    }, "unified_sort()");
    
    std::list<int> small_list = {9, 1, 8, 2, 7, 3, 6, 4, 5};
    std::forward_list<int> small_fwd_list = {9, 1, 8, 2, 7, 3, 6, 4, 5};
    
    std::cout << "\nVerify correctness:" << std::endl;
    
    print_container(small_list, "Before sort (list)");
    unified_sort(small_list.begin(), small_list.end());
    print_container(small_list, "After sort (list)");
    
    print_container(small_fwd_list, "Before sort (forward_list)");
    unified_sort(small_fwd_list.begin(), small_fwd_list.end());
    print_container(small_fwd_list, "After sort (forward_list)");
    
    return 0;
}