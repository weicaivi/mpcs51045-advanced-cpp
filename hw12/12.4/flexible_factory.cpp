#include "flexible_factory.h"
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

// Concrete model train implementations
class ModelLocomotive : public Locomotive {
    double horsepower;
public:
    ModelLocomotive(double hp) : horsepower(hp) {}
    
    void display() override {
        cout << "Model locomotive with " << horsepower << " HP" << endl;
    }
    
    double getHorsepower() const override {
        return horsepower;
    }
};

class ModelFreightCar : public FreightCar {
    long capacity;
public:
    ModelFreightCar(long cap) : capacity(cap) {}
    
    void display() override {
        cout << "Model freight car with capacity " << capacity << endl;
    }
    
    long getCapacity() const override {
        return capacity;
    }
};

class ModelCaboose : public Caboose {
public:
    ModelCaboose() {}
    
    void display() override {
        cout << "Model caboose" << endl;
    }
};

// Concrete real train implementations
class RealLocomotive : public Locomotive {
    double horsepower;
public:
    RealLocomotive(double hp) : horsepower(hp) {}
    
    void display() override {
        cout << "Real locomotive with " << horsepower << " HP" << endl;
    }
    
    double getHorsepower() const override {
        return horsepower;
    }
};

class RealFreightCar : public FreightCar {
    long capacity;
public:
    RealFreightCar(long cap) : capacity(cap) {}
    
    void display() override {
        cout << "Real freight car with capacity " << capacity << endl;
    }
    
    long getCapacity() const override {
        return capacity;
    }
};

class RealCaboose : public Caboose {
public:
    RealCaboose() {}
    
    void display() override {
        cout << "Real caboose" << endl;
    }
};

// Define the abstract factory type with constructor signatures
using TrainFactory = flexible_abstract_factory
    < Locomotive(double), 
    FreightCar(long),
    Caboose
>;

// Define concrete factories
using ModelTrainFactory = flexible_concrete_factory
    < TrainFactory, 
    ModelLocomotive, 
    ModelFreightCar, 
    ModelCaboose
>;

using RealTrainFactory = flexible_concrete_factory
    < TrainFactory, 
    RealLocomotive, 
    RealFreightCar, 
    RealCaboose
>;

int main() {
    // Create model train factory
    unique_ptr<TrainFactory> factory = make_unique<ModelTrainFactory>();
    
    // Create train components with parameters
    unique_ptr<Locomotive> locomotive = factory->create<Locomotive>(120.5);
    unique_ptr<FreightCar> freightCar = factory->create<FreightCar>(5000L);
    unique_ptr<Caboose> caboose = factory->create<Caboose>();
    
    // Display train components
    cout << "Model Train Components:" << endl;
    locomotive->display();
    freightCar->display();
    caboose->display();
    
    // Create real train factory
    factory = make_unique<RealTrainFactory>();
    
    // Create real train components with different parameters
    locomotive = factory->create<Locomotive>(5000.0);
    freightCar = factory->create<FreightCar>(50000L);
    caboose = factory->create<Caboose>();
    
    // Display train components
    cout << "\nReal Train Components:" << endl;
    locomotive->display();
    freightCar->display();
    caboose->display();
    
    return 0;
}