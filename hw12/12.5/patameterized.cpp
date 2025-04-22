#include "../12.4/flexible_factory.h"
#include <iostream>
#include <memory>
#include <string>

using namespace std;
using namespace cspp51045;

// Abstract train car classes
struct Locomotive {
    virtual void display() = 0;
    virtual double getHorsepower() const = 0;
    virtual ~Locomotive() = default;
};

struct FreightCar {
    virtual void display() = 0;
    virtual long getCapacity() const = 0;
    virtual ~FreightCar() = default;
};

struct Caboose {
    virtual void display() = 0;
    virtual ~Caboose() = default;
};

// Template for train car implementations
template<typename T>
struct Model;

template<>
struct Model<Locomotive> : public Locomotive {
    Model(double horsepower) : horsepower_(horsepower) {
        cout << "Creating model locomotive with " << horsepower << " HP" << endl;
    }
    
    void display() override {
        cout << "Model locomotive with " << horsepower_ << " HP" << endl;
    }
    
    double getHorsepower() const override {
        return horsepower_;
    }
    
private:
    double horsepower_;
};

template<>
struct Model<FreightCar> : public FreightCar {
    Model(long capacity) : capacity_(capacity) {
        cout << "Creating model freight car with " << capacity << " capacity" << endl;
    }
    
    void display() override {
        cout << "Model freight car with " << capacity_ << " capacity" << endl;
    }
    
    long getCapacity() const override {
        return capacity_;
    }
    
private:
    long capacity_;
};

template<>
struct Model<Caboose> : public Caboose {
    Model() {
        cout << "Creating model caboose" << endl;
    }
    
    void display() override {
        cout << "Model caboose" << endl;
    }
};

// Real train car implementations
template<typename T>
struct Real;

template<>
struct Real<Locomotive> : public Locomotive {
    Real(double horsepower) : horsepower_(horsepower) {
        cout << "Creating real locomotive with " << horsepower << " HP" << endl;
    }
    
    void display() override {
        cout << "Real locomotive with " << horsepower_ << " HP" << endl;
    }
    
    double getHorsepower() const override {
        return horsepower_;
    }
    
private:
    double horsepower_;
};

template<>
struct Real<FreightCar> : public FreightCar {
    Real(long capacity) : capacity_(capacity) {
        cout << "Creating real freight car with " << capacity << " capacity" << endl;
    }
    
    void display() override {
        cout << "Real freight car with " << capacity_ << " capacity" << endl;
    }
    
    long getCapacity() const override {
        return capacity_;
    }
    
private:
    long capacity_;
};

template<>
struct Real<Caboose> : public Caboose {
    Real() {
        cout << "Creating real caboose" << endl;
    }
    
    void display() override {
        cout << "Real caboose" << endl;
    }
};

// Define the abstract factory with signatures
using TrainFactory = flexible_abstract_factory<
    Locomotive(double), 
    FreightCar(long), 
    Caboose()
>;

// Parameterized factory template
template<typename AbstractFactory, template<typename> class ConcreteTemplate>
struct parameterized_factory;

// Specialized parameterized factory for TrainFactory
template<template<typename> class ConcreteTemplate>
struct parameterized_factory<TrainFactory, ConcreteTemplate> : public flexible_concrete_factory<
    TrainFactory,
    concrete_pair<TrainFactory, Locomotive(double), ConcreteTemplate<Locomotive>>,
    concrete_pair<TrainFactory, FreightCar(long), ConcreteTemplate<FreightCar>>,
    concrete_pair<TrainFactory, Caboose(), ConcreteTemplate<Caboose>>
> {};

// Define concrete factories using the parameterized approach
using ModelTrainFactory = parameterized_factory<TrainFactory, Model>;
using RealTrainFactory = parameterized_factory<TrainFactory, Real>;

int main() {
    cout << "Creating model train:" << endl;
    unique_ptr<TrainFactory> modelFactory = make_unique<ModelTrainFactory>();
    
    // Create train components with arguments
    unique_ptr<Locomotive> modelLoco = modelFactory->create<Locomotive>(75.5);
    unique_ptr<FreightCar> modelFreight = modelFactory->create<FreightCar>(250L);
    unique_ptr<Caboose> modelCaboose = modelFactory->create<Caboose>();
    
    cout << "\nDisplaying model train components:" << endl;
    modelLoco->display();
    modelFreight->display();
    modelCaboose->display();
    
    cout << "\n\nCreating real train:" << endl;
    unique_ptr<TrainFactory> realFactory = make_unique<RealTrainFactory>();
    
    unique_ptr<Locomotive> realLoco = realFactory->create<Locomotive>(12000.0);
    unique_ptr<FreightCar> realFreight = realFactory->create<FreightCar>(10000L);
    unique_ptr<Caboose> realCaboose = realFactory->create<Caboose>();
    
    cout << "\nDisplaying real train components:" << endl;
    realLoco->display();
    realFreight->display();
    realCaboose->display();
    
    return 0;
}