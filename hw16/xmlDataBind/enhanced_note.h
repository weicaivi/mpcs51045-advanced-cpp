#ifndef NOTE_H
#define NOTE_H

#include <xml/parser>
#include <string>
#include <cctype>
#include <algorithm>
#include <stdexcept>
#include <optional>
#include <sstream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <ctime>

template<typename T>
T fromXML(xml::parser &p, std::string name = "");
template<typename T> void
toXML(T const &t, xml::serializer& s, std::string name = "");

// Date class to handle xs:date
class Date {
public:
    Date() : year(0), month(0), day(0) {}
    Date(int y, int m, int d) : year(y), month(m), day(d) {}
    
    // Parse from ISO format (YYYY-MM-DD)
    static Date fromString(const std::string& dateStr) {
        Date date;
        std::istringstream ss(dateStr);
        char dash;
        ss >> date.year >> dash >> date.month >> dash >> date.day;
        if (ss.fail()) {
            throw std::runtime_error("Invalid date format: " + dateStr);
        }
        return date;
    }
    
    // Convert to ISO format
    std::string toString() const {
        std::ostringstream ss;
        ss << std::setfill('0') << std::setw(4) << year << '-'
           << std::setw(2) << month << '-'
           << std::setw(2) << day;
        return ss.str();
    }
    
    int year;
    int month;
    int day;
};

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

// Template function to reduce duplication when parsing primitive types
template<typename T>
T parseSimpleType(xml::parser &p, std::string name, std::function<T(const std::string&)> converter) {
    p.next_expect(xml::parser::start_element);
    if (!name.empty() && p.name() != name) {
        throw std::runtime_error("expected " + name + ". Got " + p.name());
    }
    
    std::string x;
    while(p.peek() == xml::parser::characters) {
        p.next();
        x += p.value();
    }
    p.next_expect(xml::parser::end_element);
    
    return converter(x);
}

// Template function to reduce duplication when serializing primitive types
template<typename T>
void serializeSimpleType(const T& value, xml::serializer &s, std::string name, std::function<std::string(const T&)> converter) {
    s.start_element(name);
    s.characters(converter(value));
    s.end_element();
}

struct note_type {
    std::string to;
    std::optional<std::string> cc;
    std::string from;
    std::string heading;
    std::string body;
    double priority;       // New field using xs:decimal
    Date sent_date;        // New field using xs:date
};

// Specialization for note_type
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
    if (p.peek() == xml::parser::start_element && p.name() == "cc") { 
        result.cc = fromXML<std::string>(p, "cc");
        munchSpace(p);
    }
    
    result.from = fromXML<std::string>(p, "from");
    munchSpace(p);
    
    result.heading = fromXML<std::string>(p, "heading");
    munchSpace(p);
    
    result.body = fromXML<std::string>(p, "body");
    munchSpace(p);
    
    result.priority = fromXML<double>(p, "priority");
    munchSpace(p);
    
    result.sent_date = fromXML<Date>(p, "sent_date");
    munchSpace(p);
    
    p.next_expect(xml::parser::end_element);
    return result;
}

template<>
void toXML<note_type>(note_type const& x, xml::serializer &s, std::string name) {
    if(name.empty()) name = "note";
    s.start_element(name);
    
    toXML(x.to, s, "to");
    
    if (x.cc) {
        toXML(x.cc.value(), s, "cc");
    }
    
    toXML(x.from, s, "from");
    toXML(x.heading, s, "heading");
    toXML(x.body, s, "body");
    toXML(x.priority, s, "priority");
    toXML(x.sent_date, s, "sent_date");
    
    s.end_element();
}

// String type
template<>
std::string fromXML<std::string>(xml::parser &p, std::string name) {
    return parseSimpleType<std::string>(p, name, [](const std::string& s) { return s; });
}

template<>
void toXML<std::string>(std::string const& x, xml::serializer &s, std::string name) {
    serializeSimpleType<std::string>(x, s, name, [](const std::string& s) { return s; });
}

// Boolean type
template<>
bool fromXML<bool>(xml::parser &p, std::string name) {
    return parseSimpleType<bool>(p, name, [](const std::string& s) { return s == "true" ? true : false; });
}

template<>
void toXML<bool>(bool const& x, xml::serializer &s, std::string name) {
    serializeSimpleType<bool>(x, s, name, [](const bool& b) { return b ? "true" : "false"; });
}

// Integer type
template<>
int fromXML<int>(xml::parser &p, std::string name) {
    return parseSimpleType<int>(p, name, [](const std::string& s) { return std::stoi(s); });
}

template<>
void toXML<int>(int const& x, xml::serializer &s, std::string name) {
    serializeSimpleType<int>(x, s, name, [](const int& i) { return std::to_string(i); });
}

// Byte (signed char) type
template<>
signed char fromXML<signed char>(xml::parser &p, std::string name) {
    return parseSimpleType<signed char>(p, name, [](const std::string& s) { return static_cast<signed char>(std::stoi(s)); });
}

template<>
void toXML<signed char>(signed char const& x, xml::serializer &s, std::string name) {
    serializeSimpleType<signed char>(x, s, name, [](const signed char& c) { return std::to_string(c); });
}

// Double (decimal) type - NEW TYPE
template<>
double fromXML<double>(xml::parser &p, std::string name) {
    return parseSimpleType<double>(p, name, [](const std::string& s) { return std::stod(s); });
}

template<>
void toXML<double>(double const& x, xml::serializer &s, std::string name) {
    serializeSimpleType<double>(x, s, name, [](const double& d) { 
        std::ostringstream ss;
        ss << std::fixed << d;
        return ss.str();
    });
}

// Date type - NEW TYPE
template<>
Date fromXML<Date>(xml::parser &p, std::string name) {
    return parseSimpleType<Date>(p, name, [](const std::string& s) { return Date::fromString(s); });
}

template<>
void toXML<Date>(Date const& x, xml::serializer &s, std::string name) {
    serializeSimpleType<Date>(x, s, name, [](const Date& d) { return d.toString(); });
}

#endif // NOTE_H