#ifndef OSTREAM_JOINER_H
#  define OSTREAM_JOINER_H
#define _SCL_SECURE_NO_WARNINGS

#include <iostream>
#include <string>
#include <iterator>
#include <cstddef>
using std::basic_ostream;
using std::ostream;
using std::string;
using std::basic_string;
using std::char_traits;
using std::allocator;


namespace mpcs {
struct my_do_output {
    my_do_output(ostream &os, string del)
        : first(true), os(&os), del(del) {
    }

    my_do_output(my_do_output const&) = default;

    template<typename T>
    my_do_output &operator=(T const& t) {
        if (!first) {
            *os << del;
        }
        first = false;
        *os << t;
        return *this;
    }
private:
    bool first;
    ostream *os;
    string del;
};


struct ostream_joiner {
    using iterator_category = std::output_iterator_tag;
    using iterator_concept = std::output_iterator_tag;
    using value_type = void;
    using difference_type = std::ptrdiff_t;
    using pointer = void;
    using reference = void;

    ostream_joiner(ostream& os, string del) : mdo(os, del) {}

    my_do_output& operator *() { return mdo;  }
    ostream_joiner& operator++() { return *this; }
    ostream_joiner& operator++(int) { return *this; }
    my_do_output mdo;
};


}
#endif