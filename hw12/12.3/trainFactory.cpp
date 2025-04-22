#include "factory.h"
#include <iostream>
#include <memory>
using namespace std;
using namespace cspp51045;

// Abstract train car classes
struct Locomotive {
    virtual void display() = 0;
    virtual ~Locomotive() = default;
};

struct FreightCar {
    virtual void display() = 0;
    virtual ~FreightCar() = default;
};

struct Caboose {
    virtual void display() = 0;
    virtual ~Caboose() = default;
};

// Concrete model train implementations
struct ModelLocomotive : public Locomotive {
    void display() override {
        cout << "Model locomotive" << endl;
    }
};

struct ModelFreightCar : public FreightCar {
    void display() override {
        cout << "Model freight car" << endl;
    }
};

struct ModelCaboose : public Caboose {
    void display() override {
        cout << "Model caboose" << endl;
    }
};

// Concrete real train implementations
struct RealLocomotive : public Locomotive {
    void display() override {
        cout << "Real locomotive" << endl;
    }
};

struct RealFreightCar : public FreightCar {
    void display() override {
        cout << "Real freight car" << endl;
    }
};

struct RealCaboose : public Caboose {
    void display() override {
        cout << "Real caboose" << endl;
    }
};

// Define the abstract factory type
using TrainFactory = abstract_factory<Locomotive, FreightCar, Caboose>;

// Define concrete factories
using ModelTrainFactory = 
    concrete_factory<TrainFactory, ModelLocomotive, ModelFreightCar, ModelCaboose>;

using RealTrainFactory = 
    concrete_factory<TrainFactory, RealLocomotive, RealFreightCar, RealCaboose>;

int main() {
    unique_ptr<TrainFactory> factory = make_unique<ModelTrainFactory>();
    
    unique_ptr<Locomotive> locomotive(factory->create<Locomotive>());
    unique_ptr<FreightCar> freightCar(factory->create<FreightCar>());
    unique_ptr<Caboose> caboose(factory->create<Caboose>());
    
    locomotive->display();
    freightCar->display();
    caboose->display();
    
    return 0;
}