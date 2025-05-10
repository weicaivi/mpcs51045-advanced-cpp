#ifndef XMLBIND_H
#  define XMLBIND_H
#include<type_traits>
#include<stack>
#include<variant>
#include<string>
#include<functional>
#include<string_view>
#include<map>
#include<vector>
#include<memory>
#include<utility>
#include<iostream>
#include <xml/parser>
using xml::parser;
using std::index_sequence;
using std::make_index_sequence;
using std::variant;
using std::stack;
using std::string;
using std::reference_wrapper;
using std::ref;
using std::string_view;
using std::function;
using std::less;
using std::map;
using std::vector;
using std::make_unique;
using std::unique_ptr;
using std::pair;
using std::istream;
using std::ostream;
using std::forward;
using std::is_base_of_v;
using std::is_same_v;
using namespace std::string_literals;

namespace mpcs {
namespace v1 {
template<typename T>
struct single_visitor {
    virtual void visit(T const&) const {}
};

template<typename T> struct visitor_helper;
template<typename ...Ts>
struct visitor_helper<typelist<Ts...>> : public single_visitor<Ts>... {
    using single_visitor<Ts>::visit...;
};

template<parser::event_type e>
struct event_t {
    static parser::event_type constexpr event = e;
};

// Some ugly boilerplate because we can only use parameter
// packs inside templates. P2858 proposes to remove this.
template<typename T> struct EVHelper;
template<size_t... nums>
struct EVHelper<index_sequence<nums...>> {
    using type = variant<event_t<parser::event_type(nums)>...>;
};
using event_variant = EVHelper<make_index_sequence<parser::eof + 1>>::type;

struct event_holder : public event_variant {
    using event_variant::event_variant;
    inline event_holder(parser::event_type et);
};

template<typename ...Ts>
event_holder event_to_variant_helper(parser::event_type e, variant<Ts...>&&)
{
    static map<parser::event_type, event_holder> val_to_type = {
        {Ts::event, event_t<Ts::event>()}...
    };
    return val_to_type[e];
}

event_holder event_to_variant(parser::event_type e)
{
    return event_to_variant_helper(e, event_variant());
}

event_holder::event_holder(parser::event_type et)
    : event_variant(event_to_variant(et)) {}

template<typename T>
struct processor {
    virtual void doProcess(T const&) {}
    virtual ~processor() = default;
};

template<typename T> struct event_processor_helper; 
using event_processor = event_processor_helper<event_variant>;
stack<unique_ptr<event_processor>> eventProcessors;

template <typename ...Ts>
struct event_processor_helper<variant<Ts...>> : public processor<Ts>... {

    using processor<Ts>::doProcess...; // Needed because of https://godbolt.org/z/xjhnx5G7b
 
    event_processor_helper(parser& p) : p(p) {}

    void process(parser::event_type e) {
        visit([this](auto const& x) {
            doProcess(x);
            afterProcess(x);
            }, 
            static_cast<event_variant&&>(event_holder(e))); // C++20 only allows variants to be visited
//          event_holder(e)); // C++23 will allows class that inherit from variants to be visited
    }

    template<parser::event_type e>
    void afterProcess(event_t<e> const&) {}
    void afterProcess(event_t<parser::start_element> const &);

    void afterProcess(event_t<parser::end_element> const &) {
        eventProcessors.pop();
    }
    parser& p;
};

struct elt : public event_processor {
    using event_processor::event_processor;
};

struct schema_elt : public elt {
    using elt::elt;
};

struct element_elt : public elt {
    element_elt(parser& p);
    string name = p.attribute("name");
    string typeName = p.attribute("type", defaultTypeName());
    bool multiple = (p.attribute_present("maxOccurs") 
                     && (p.attribute("maxOccurs") == "unbounded" || p.attribute<int>("maxOccurs") > 1));
    bool optional = !multiple && (p.attribute("minOccurs", 1) == 0) && p.attribute("maxOccurs", 1) == 1;
    ~element_elt();
    string defaultTypeName() { return name + "_type"s; }
};

struct sequence_elt : public elt {
    using elt::elt;
};

struct complex_type_elt : public elt {
    complex_type_elt(parser& p);
    ~complex_type_elt();
};

stack<reference_wrapper<element_elt>> elementStack;
element_elt& cur_element() { return elementStack.top().get(); }

// Note: less<> for heterogeneous lookup for better semantics
unique_ptr<elt> elt_from_name(string_view name, parser& p) {
    static map<string, function<unique_ptr<elt>(parser&)>, less<>> makers = {
        {"schema", make_unique<schema_elt, parser&>},
        {"element", make_unique<element_elt, parser&>},
        {"sequence", make_unique<sequence_elt, parser&>},
        {"complexType", make_unique<complex_type_elt, parser&>}
    };
    return (makers.find(name)->second)(p);
}

template <typename ...Ts>
void event_processor_helper<variant<Ts...>>::afterProcess(event_t<parser::start_element> const &)
{
    eventProcessors.push(elt_from_name(p.name(), p));
}

struct data_member;
struct optional_data_member;
struct multiple_data_member;
struct type_base;
struct global_scope;
struct complex_type;
struct builtin_type;
struct string_type;
struct int_type;
struct boolean_type;
struct byte_type;

using all_data_members = typelist<data_member, optional_data_member, multiple_data_member>;
using data_member_visitor = visitor_helper<all_data_members>;

struct data_member { // Aggregate
    data_member(string name, type_base& type) : name(name), type(type) {}
    virtual ~data_member() = default;
    virtual void accept(data_member_visitor const& v) const { v.visit(*this); }
    string name;
    type_base& type;
};

struct optional_data_member : public data_member {
    using data_member::data_member;
    virtual void accept(data_member_visitor const& v) const { v.visit(*this); }
};

struct multiple_data_member : public data_member {
    using data_member::data_member;
    virtual void accept(data_member_visitor const& v) const { v.visit(*this); }
};

using all_types = typelist<type_base, global_scope, complex_type, builtin_type, string_type, int_type, byte_type, boolean_type>;
using type_visitor = visitor_helper<all_types>;

struct scope {
    map<string, unique_ptr<type_base>> memberTypes;
    vector<unique_ptr<data_member>> dataMembers;
};

struct type_base {
    virtual void accept(type_visitor const& tv) const { tv.visit(*this); }
    type_base() = default;
    type_base(string name) : name(name) {}
    virtual ~type_base() = default;
    string name;
};

struct builtin_type : public type_base {
    virtual void accept(type_visitor const& tv) const { tv.visit(*this); }
    builtin_type(string name, string xsdName) : type_base(name), xsdName(xsdName) {}
    string xsdName;
};

struct string_type : public builtin_type {
    virtual void accept(type_visitor const& tv) const { tv.visit(*this); }
    string_type() : builtin_type("std::string", "xs:string") {}
};

struct int_type : public builtin_type {
  virtual void accept(type_visitor const& tv) const { tv.visit(*this); }
  int_type() : builtin_type("int", "xs:int") {}
};

struct boolean_type : public builtin_type {
  virtual void accept(type_visitor const& tv) const { tv.visit(*this); }
  boolean_type() : builtin_type("bool", "xs:boolean") {}
};

struct byte_type : public builtin_type {
    virtual void accept(type_visitor const& tv) const { tv.visit(*this); }
    byte_type() : builtin_type("signed char", "xs:byte") {}
};

struct complex_type : virtual public type_base, public scope {
    virtual void accept(type_visitor const& tv) const { tv.visit(*this); }
    complex_type(parser& p) 
      : type_base(p.attribute("name", cur_element().defaultTypeName())),
        containingElementName(!elementStack.empty()? cur_element().name : ""),
        anonymous(p.attribute_present("name")){}
    string containingElementName;
    bool anonymous;
protected:
    complex_type() {}
};

#if 0
struct global_scope : virtual public complex_type {
    global_scope() {
        memberTypes["xs:string"] = make_unique<string_type>();
        memberTypes["xs:int"] = make_unique<int_type>();
        memberTypes["xs:byte"] = make_unique<byte_type>();
        memberTypes["xs:boolean"] = make_unique<boolean_type>();
    }
    virtual void accept(type_visitor const & tv) const { tv.visit(*this); }
};

#else
template<typename T>
struct derives {
  template<typename U>
  struct from {
    static bool constexpr value = !is_same_v<U, T> && is_base_of_v<T, U>;
  };
};


struct global_scope : virtual public complex_type {
  inline global_scope();
  template<typename ...Ts>
  void populate(Type2Type<typelist<Ts...>>&&);
  virtual void accept(type_visitor const& tv) const { tv.visit(*this); }
};

// Typelist of the builtin types. E.g., typelist<string_type, int_type,...
using builtin_types = Filter_t<all_types, derives<builtin_type>::from>;
inline global_scope::global_scope() {
  populate(Type2Type<builtin_types>());
}

template<typename ...Ts>
void global_scope::populate(Type2Type<typelist<Ts...>>&&) {
  (... + (([this](auto x) { memberTypes[x->xsdName] = move(x); return 1; })(make_unique<Ts>())));
}

#endif
vector<reference_wrapper<scope>> scopeStack;

type_base& findType(string name)
{
    for (auto it = scopeStack.rbegin(); it != scopeStack.rend(); it++) {
        auto lookup = it->get().memberTypes.find(name);
        if (lookup != it->get().memberTypes.end())
            return *lookup->second;
    }
    // Todo: Handle error
}

element_elt::element_elt(parser& p)
    : elt(p)
{
    elementStack.push(*this);
}

element_elt::~element_elt()
{
    unique_ptr<data_member> newDM;
    if (optional)
        newDM = make_unique<optional_data_member>(name, findType(typeName));
    else if (multiple)
        newDM = make_unique<multiple_data_member>(name, findType(typeName));
    else
        newDM = make_unique<data_member>(name, findType(typeName));

    // Transferring ownership. Use move()
    scopeStack.back().get().dataMembers.push_back(move(newDM));
    elementStack.pop();
}

complex_type_elt::complex_type_elt(parser& p)
    : elt(p)
{
    auto newType = make_unique<complex_type>(p);
    scope& newScope = *newType;
    scopeStack.back().get().memberTypes.insert(pair(newType->name, move(newType)));
    scopeStack.push_back(newScope);
}

complex_type_elt::~complex_type_elt()
{
    scopeStack.pop_back();
}
}
using namespace v1;
}

#endif
