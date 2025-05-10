#ifndef STRUCT_FORMATTER_H
#  define STRUCT_FORMATTER_H
#include <sstream>
#include "formatter.h"
#include "serializer_decl_formatter.h"
#include "IndentStream.h"
#include "fmt/format.h"
using fmt::format;
using std::ostringstream;

namespace mpcs {
namespace v1 {
// Don't output the deserializers immediately but instead
// write them to an ostringstream, so they can be output
// at the end when everything they use has been defined
static ostringstream cachedDeserializersString;
static IndentStream cachedDeserializers(cachedDeserializersString);
inline generate_args gaForDeserialization(generate_args f)
{
    return { f.factory, cachedDeserializers };
}

template<typename T> struct struct_formatter;
template<typename T> 
struct struct_formatter_bases 
  : public inheriter<struct_formatter, direct_bases_t<T, all_types>>, virtual public formatter<T> {
    virtual void generate(generate_args& f, T const&) {}
    virtual ~struct_formatter_bases() = default;
};

template<typename T>
struct struct_formatter : public struct_formatter_bases<T> {
};

template<>
struct struct_formatter<complex_type> : public struct_formatter_bases<complex_type> {

    struct data_member_formatter {
        data_member_formatter(data_member const& dm) : dm(dm) {}

        virtual string typeName() const { return dm.type.name; }

        virtual void generateSerializer(generate_args& fa) const {
          fa.os << format("toXML(x.{}, s, \"{}\");\n", dm.name, dm.name);
        }

        virtual void generateDeserializer(generate_args& fa) const {
            fa.os << format("result.{} = fromXML<{}>(p, \"{}\");\n",
                dm.name, dm.type.name, dm.name);
        }

        data_member const& dm;
    };

    struct optional_data_member_formatter : public data_member_formatter {
        optional_data_member_formatter(optional_data_member const& odm) 
            : data_member_formatter(odm), odm(odm) {}

        virtual string typeName() const {
            return format("std::optional<{}>", odm.type.name);
        }

        virtual void generateSerializer(generate_args& fa) const override {
          fa.os << format(
            R"(if (x.{0})
    toXML(x.{0}.value(), s, "{0}");
)", odm.name);
        }

        virtual void generateDeserializer(generate_args& fa) const override {
            fa.os << format(
R"(if (auto e = p.peek() != xml::parser::start_element) {{
    std::ostringstream error; // Use stream to leverage << for event_types
    error << "Expected start element. Got " << e;
    throw std::runtime_error(error.str());
}}
if(p.name() == "{}") {{ 
)", dm.name) << indent;
            data_member_formatter::generateDeserializer(fa);
            fa.os << unindent << "}\n";
        }

        optional_data_member const& odm;
    };

    struct multiple_data_member_formatter : public data_member_formatter {
        multiple_data_member_formatter(multiple_data_member const& mdm)
            : data_member_formatter(mdm), mdm(mdm) {}

        virtual string typeName() const {
            return format("std::vector<{}>", mdm.type.name);
        }

        virtual void generateSerializer(generate_args& fa) const override {
          fa.os << format(
            R"(for (auto const &m : x.{0})
    toXML(m, s, "{0}");
)", mdm.name);
        }

        virtual void generateDeserializer(generate_args& fa) const override {
            fa.os << format(
R"(while (p.peek() == xml::parser::start_element && p.name() == "{0}") {{
    result.{0}.push_back(fromXML<{1}>(p, "{0}"));
}}
)", mdm.name, mdm.type.name);
        }

        multiple_data_member const& mdm;
    };

    struct dm_formatter_factory_visitor : data_member_visitor {
        dm_formatter_factory_visitor(unique_ptr<data_member_formatter>& up) : up(up) {}
        void visit(data_member const &dm) const override { up = make_unique<data_member_formatter>(dm); }
        void visit(optional_data_member const& odm) const override { up = make_unique<optional_data_member_formatter>(odm); }
        void visit(multiple_data_member const& odm) const override { up = make_unique<multiple_data_member_formatter>(odm); }
        unique_ptr<data_member_formatter> &up;
    };

    unique_ptr<data_member_formatter> dmFormatter(data_member const& dm) {
        unique_ptr<data_member_formatter> result;
        dm.accept(dm_formatter_factory_visitor(result));
        return result;
    }

    void generate_begin(generate_args& f, complex_type const& ct) {
        f.os << format("struct {} {{\n", ct.name)
             << indent;
    }
    void generate_member_types(generate_args& f, complex_type const& ct) {
        for (auto& memberType : ct.memberTypes) {
            memberType.second->accept(formatter_visitor(f));
        }
    }

    void generate_data_members(generate_args& f, complex_type const& ct) {
        for (auto& dm : ct.dataMembers) {
            f.os << dmFormatter(*dm)->typeName() << " " << dm->name << ";\n";
        }
    }

    void generate_members(generate_args& f, complex_type const& ct) {
        generate_member_types(f, ct);
        generate_data_members(f, ct);
    }
    void generate_end(generate_args& f, complex_type const& ct) {
        f.os << unindent << "};\n\n";
    }

    void generate_serializer(generate_args& f, complex_type const& ct) {
      f.os << serializer_specialization(ct) << " {\n" << indent;
      if (!ct.anonymous)
        f.os << format("if(name.empty()) name = \"{}\";\n", ct.containingElementName);
      f.os << "s.start_element(name);\n";
      for (auto& dm : ct.dataMembers) {
        dmFormatter(*dm)->generateSerializer(f);
      }
      f.os << "s.end_element();\n";
      f.os << unindent << "}\n";
    }

    void generate_deserializer(generate_args& f, complex_type const& ct) {
        f.os << deserializer_specialization(ct) << " {\n" << indent; 
        if(!ct.anonymous)
            f.os << format("if(name.empty()) name = \"{}\";\n", ct.containingElementName);
        f.os << format(
R"({} result;
p.next_expect(xml::parser::start_element);
if(p.name() != name)
    throw std::runtime_error("expected " + name + ". Got " + p.name());
)", ct.name);

        for (auto& dm : ct.dataMembers) {
            f.os << "munchSpace(p);\n";
            dmFormatter(*dm)->generateDeserializer(f);
        }
        f.os << 
R"(munchSpace(p);
p.next_expect(xml::parser::end_element);
return result;
)"; 
        f.os << unindent << "}\n\n";
    }

    virtual void generate(generate_args& f, complex_type const &ct) override {
        generate_begin(f, ct);
        generate_members(f, ct);
        generate_end(f, ct);
        generate_args fa = gaForDeserialization(f);
        generate_serializer(fa, ct);
        generate_deserializer(fa, ct);
    }
};

// Class note: I forgot to put in the override here and had to debug a lot. Given how nasty template debugging is, we would rather the compiler work for us
template<>
struct struct_formatter<global_scope> : public struct_formatter_bases<global_scope> {
    unique_ptr<formatter_factory> declFactory = std::make_unique<serializer_decl_formatter_factory>();

    void outputHeader(ostream& os) {
        os <<
R"(// Generated XML Schema Binding file
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
)";
    }

    virtual void generate(generate_args &f, global_scope const& ct) override {
        cachedDeserializersString.str(""); // Clear the cached declarations

        outputHeader(f.os);
        generate_member_types(f, ct);
        generate_args fa{ *declFactory, f.os };
        ct.accept(formatter_visitor(fa));
        f.os << "\n\n" << cachedDeserializersString.str();
    }
};

template<>
struct struct_formatter<builtin_type> : public struct_formatter_bases<builtin_type> {
    virtual string xToChars() const { return ""; }
    virtual string charsToX() const { return ""; }

    void generate_serializer(generate_args& f, builtin_type const& s) {
      f.os << serializer_specialization(s) << " {\n" << indent;
      f.os << format(
        R"(s.start_element(name);
s.characters({}),
s.end_element();
)", xToChars()) << unindent;
      f.os << "}\n";
    }

    void generate_deserializer(generate_args& f, builtin_type const& s) {
        f.os << format("template<>\n{0} fromXML<{0}>(xml::parser &p, std::string name) {{\n", s.name);
        f.os << indent << format(
R"(p.next_expect(xml::parser::start_element);
std::string x;
while(p.peek() == xml::parser::characters) {{
    p.next();
    x += p.value();
}}
p.next_expect(xml::parser::end_element);
return {};        
)", charsToX());
        f.os << unindent << "}\n\n";
    }
    void doGenerate(generate_args& f, builtin_type const& s) {
        generate_args fa = gaForDeserialization(f);
        generate_deserializer(fa, s);
        generate_serializer(fa, s);
    }
};

template<>
struct struct_formatter<string_type> : public struct_formatter_bases<string_type> {
    virtual string xToChars() const override { return "x"; }
    virtual string charsToX() const override { return "x"; }
    virtual void generate(generate_args& f, string_type const& s) override { doGenerate(f, s); }
};


template<>
struct struct_formatter<int_type> : public struct_formatter_bases<int_type> {
  virtual string xToChars() const override { return "std::to_string(x)"; }
  virtual string charsToX() const override { return "std::stoi(x)"; }
  virtual void generate(generate_args& f, int_type const& s) override { doGenerate(f, s); }

};

template<>
struct struct_formatter<boolean_type> : public struct_formatter_bases<boolean_type> {
  virtual string xToChars() const override { return R"(x ? "true" : "false")"; }
  virtual string charsToX() const override { return R"(x == "true"? true : false)"; }
  virtual void generate(generate_args& f, boolean_type const& s) override { doGenerate(f, s); }

};

template<>
struct struct_formatter<byte_type> : public struct_formatter_bases<byte_type> {
    virtual string xToChars() const override { return "std::to_string(x)"; }
    virtual string charsToX() const override { return "std::stoi(x)"; }
    virtual void generate(generate_args& f, byte_type const& s) override { doGenerate(f, s); }

};

using struct_formatter_factory = parallel_concrete_factory<formatter_factory, struct_formatter>;

}
}
#endif
