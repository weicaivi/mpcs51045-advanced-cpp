#ifndef FLEXIBLE_FACTORY_H
#define FLEXIBLE_FACTORY_H
#include <tuple>
#include <memory>
#include <type_traits>

namespace cspp51045 {

// Forward declarations
template<typename T>
struct creator_base;

template<typename R, typename... Args>
struct creator_with_args;

// Base class for all creators with default constructor
template<typename T>
struct creator_base {
    virtual std::unique_ptr<T> create() = 0;
    virtual ~creator_base() = default;
};

// Base class for creators with parameterized constructor
template<typename R, typename... Args>
struct creator_with_args {
    virtual std::unique_ptr<R> create(Args... args) = 0;
    virtual ~creator_with_args() = default;
};

// The abstract factory
template<typename... Types>
struct flexible_abstract_factory;

// Base case for recursion
template<>
struct flexible_abstract_factory<> {
    virtual ~flexible_abstract_factory() = default;
};

// Specialization for regular types
template<typename First, typename... Rest>
struct flexible_abstract_factory<First, Rest...> 
    : public creator_base<First>, 
      public flexible_abstract_factory<Rest...> {};

// Specialization for function signature types
template<typename R, typename... Args, typename... Rest>
struct flexible_abstract_factory<R(Args...), Rest...> 
    : public creator_with_args<R, Args...>, 
      public flexible_abstract_factory<Rest...> {};

// Concrete creator for regular types
template<typename AbstractFactory, typename Abstract, typename Concrete>
struct concrete_creator : virtual public AbstractFactory {
    std::unique_ptr<Abstract> create() override {
        return std::make_unique<Concrete>();
    }
};

// Concrete creator for parameterized types
template<typename AbstractFactory, typename Concrete, typename R, typename... Args>
struct concrete_creator_with_args : virtual public AbstractFactory {
    std::unique_ptr<R> create(Args... args) override {
        return std::make_unique<Concrete>(std::forward<Args>(args)...);
    }
};

// Helper to determine if a type is a function signature
template<typename T>
struct is_signature : std::false_type {};

template<typename R, typename... Args>
struct is_signature<R(Args...)> : std::true_type {};

// Helper to extract return type from a function signature
template<typename T>
struct return_type {
    using type = T;
};

template<typename R, typename... Args>
struct return_type<R(Args...)> {
    using type = R;
};

// Concrete factory implementation
template<typename AbstractFactory, typename... AbstractConcretePairs>
struct flexible_concrete_factory;

// Base case
template<typename AbstractFactory>
struct flexible_concrete_factory<AbstractFactory> : public AbstractFactory {};

// Implementation for regular types
template<typename AbstractFactory, typename Abstract, typename Concrete, typename... Rest>
struct flexible_concrete_factory
    AbstractFactory, 
    Abstract, Concrete, 
    Rest...
> : public concrete_creator<AbstractFactory, Abstract, Concrete>,
    public flexible_concrete_factory<AbstractFactory, Rest...> {
    
    // Constructor forwarding
    using concrete_creator<AbstractFactory, Abstract, Concrete>::create;
    using flexible_concrete_factory<AbstractFactory, Rest...>::create;
};

// Implementation for function signature types
template<typename AbstractFactory, typename R, typename... Args, typename Concrete, typename... Rest>
struct flexible_concrete_factory
    AbstractFactory, 
    R(Args...), Concrete, 
    Rest...
> : public concrete_creator_with_args<AbstractFactory, Concrete, R, Args...>,
    public flexible_concrete_factory<AbstractFactory, Rest...> {
    
    // Constructor forwarding
    using concrete_creator_with_args<AbstractFactory, Concrete, R, Args...>::create;
    using flexible_concrete_factory<AbstractFactory, Rest...>::create;
};

// Helper to zip abstract types with concrete types
template<typename AbstractFactory, typename... AbstractTypes>
struct factory_implementation;

// Implementation of the abstract-concrete type pairing
template<typename... AbstractTypes, typename... ConcreteTypes>
struct factory_implementation<flexible_abstract_factory<AbstractTypes...>, ConcreteTypes...> {
    using type = flexible_concrete_factory
        flexible_abstract_factory<AbstractTypes...>,
        AbstractTypes, ConcreteTypes...
    >;
};

// Final concrete factory definition
template<typename AbstractFactory, typename... ConcreteTypes>
using concrete_factory = typename factory_implementation<AbstractFactory, ConcreteTypes...>::type;

} // namespace cspp51045
#endif