#ifndef FACTORY_H
#define FACTORY_H
#include<tuple>
#include<memory>
using std::tuple;
using std::unique_ptr;
using std::make_unique;
#pragma warning(disable : 4250)

namespace cspp51045 {
template<typename T>
struct TT {
};

template<typename T>
struct abstract_creator {
    virtual unique_ptr<T> doCreate(TT<T> &&) = 0;
};

template<typename... Ts>
struct abstract_factory : public abstract_creator<Ts>... {

	template<class U> unique_ptr<U> create() {
		abstract_creator<U> &creator = *this;
		return creator.doCreate(TT<U>());
	}
	virtual ~abstract_factory() = default;
};

template<typename AbstractFactory, typename Abstract, typename Concrete>
struct concrete_creator : virtual public AbstractFactory {
	unique_ptr<Abstract> doCreate(TT<Abstract> &&) override {
		return make_unique<Concrete>();
	}
};

template<typename AbstractFactory, typename... ConcreteTypes>
struct concrete_factory;

template<typename... AbstractTypes, typename... ConcreteTypes>
struct concrete_factory
  <abstract_factory<AbstractTypes...>, ConcreteTypes...> 
  : public concrete_creator<abstract_factory<AbstractTypes...>, 
	                        AbstractTypes, ConcreteTypes>... {
};
}
#endif
