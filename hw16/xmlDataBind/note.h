// Generated XML Schema Binding file
// UChicago MPCS51045

#include <xml/parser>
#include <string>
#include <cctype>
#include <algorithm>
#include <stdexcept>
#include <optional>
#include <sstream>
#include <vector>

template<typename T>
T fromXML(xml::parser &p, std::string name = "");
template<typename T> void
toXML(T const &t, xml::serializer& s, std::string name = "");

// skip over whitespace in XML file
void munchSpace(xml::parser &p)
{
    while(p.peek() == p.characters) {
        p.next();
        auto s = p.value();
        if(!std::all_of(s.begin(),s.end(),static_cast<int(*)(int)>(std::isspace)))
            throw std::runtime_error("Unexpected characters: " + s);
    }
}
struct note_type {
    std::string to;
    std::optional<std::string> cc;
    std::string from;
    std::string heading;
    std::string body;
};

template<>
note_type fromXML<note_type>(xml::parser &p, std::string name);
template<>
void toXML<note_type>(note_type const& x, xml::serializer &s, std::string name);
template<>
bool fromXML<bool>(xml::parser &p, std::string name);
template<>
void toXML<bool>(bool const& x, xml::serializer &s, std::string name);
template<>
signed char fromXML<signed char>(xml::parser &p, std::string name);
template<>
void toXML<signed char>(signed char const& x, xml::serializer &s, std::string name);
template<>
int fromXML<int>(xml::parser &p, std::string name);
template<>
void toXML<int>(int const& x, xml::serializer &s, std::string name);
template<>
std::string fromXML<std::string>(xml::parser &p, std::string name);
template<>
void toXML<std::string>(std::string const& x, xml::serializer &s, std::string name);


template<>
void toXML<note_type>(note_type const& x, xml::serializer &s, std::string name) {
    if(name.empty()) name = "note";
    s.start_element(name);
    toXML(x.to, s, "to");
    if (x.cc)
        toXML(x.cc.value(), s, "cc");
    toXML(x.from, s, "from");
    toXML(x.heading, s, "heading");
    toXML(x.body, s, "body");
    s.end_element();
}
template<>
note_type fromXML<note_type>(xml::parser &p, std::string name) {
    if(name.empty()) name = "note";
    note_type result;
    p.next_expect(xml::parser::start_element);
    if(p.name() != name)
        throw std::runtime_error("expected " + name + ". Got " + p.name());
    munchSpace(p);
    result.to = fromXML<std::string>(p, "to");
    munchSpace(p);
    if (auto e = p.peek() != xml::parser::start_element) {
        std::ostringstream error; // Use stream to leverage << for event_types
        error << "Expected start element. Got " << e;
        throw std::runtime_error(error.str());
    }
    if(p.name() == "cc") { 
        result.cc = fromXML<std::string>(p, "cc");
    }
    munchSpace(p);
    result.from = fromXML<std::string>(p, "from");
    munchSpace(p);
    result.heading = fromXML<std::string>(p, "heading");
    munchSpace(p);
    result.body = fromXML<std::string>(p, "body");
    munchSpace(p);
    p.next_expect(xml::parser::end_element);
    return result;
}

template<>
bool fromXML<bool>(xml::parser &p, std::string name) {
    p.next_expect(xml::parser::start_element);
    std::string x;
    while(p.peek() == xml::parser::characters) {
        p.next();
        x += p.value();
    }
    p.next_expect(xml::parser::end_element);
    return x == "true"? true : false;        
}

template<>
void toXML<bool>(bool const& x, xml::serializer &s, std::string name) {
    s.start_element(name);
    s.characters(x ? "true" : "false"),
    s.end_element();
}
template<>
signed char fromXML<signed char>(xml::parser &p, std::string name) {
    p.next_expect(xml::parser::start_element);
    std::string x;
    while(p.peek() == xml::parser::characters) {
        p.next();
        x += p.value();
    }
    p.next_expect(xml::parser::end_element);
    return std::stoi(x);        
}

template<>
void toXML<signed char>(signed char const& x, xml::serializer &s, std::string name) {
    s.start_element(name);
    s.characters(std::to_string(x)),
    s.end_element();
}
template<>
int fromXML<int>(xml::parser &p, std::string name) {
    p.next_expect(xml::parser::start_element);
    std::string x;
    while(p.peek() == xml::parser::characters) {
        p.next();
        x += p.value();
    }
    p.next_expect(xml::parser::end_element);
    return std::stoi(x);        
}

template<>
void toXML<int>(int const& x, xml::serializer &s, std::string name) {
    s.start_element(name);
    s.characters(std::to_string(x)),
    s.end_element();
}
template<>
std::string fromXML<std::string>(xml::parser &p, std::string name) {
    p.next_expect(xml::parser::start_element);
    std::string x;
    while(p.peek() == xml::parser::characters) {
        p.next();
        x += p.value();
    }
    p.next_expect(xml::parser::end_element);
    return x;        
}

template<>
void toXML<std::string>(std::string const& x, xml::serializer &s, std::string name) {
    s.start_element(name);
    s.characters(x),
    s.end_element();
}
