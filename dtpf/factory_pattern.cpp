// factory_pattern.cpp
// Task base classes and factory implementation

#include <memory>
#include <string>
#include <map>
#include <functional>
#include <stdexcept>

namespace dtpf {

// Forward declarations from meta_programming.cpp
template<typename T>
concept TaskResult = requires(T t) {
    { t.serialize() } -> std::convertible_to<std::string>;
    { T::deserialize(std::string{}) } -> std::same_as<T>;
    std::movable<T>;
};

// ============================================================================
// TASK BASE CLASSES
// ============================================================================

class TaskBase {
public:
    virtual ~TaskBase() = default;
    virtual std::string execute() = 0;
    virtual std::string get_type() const = 0;
    virtual int get_priority() const = 0;
};

template<TaskResult R>
class Task : public TaskBase {
public:
    virtual R execute_typed() = 0;
    
    std::string execute() override {
        return execute_typed().serialize();
    }
};

// ============================================================================
// FACTORY PATTERN IMPLEMENTATION
// ============================================================================

class TaskFactory {
public:
    using TaskCreator = std::function<std::unique_ptr<TaskBase>(const std::string&)>;
    
    template<typename T>
    void register_task(const std::string& type_name) {
        static_assert(std::is_base_of_v<TaskBase, T>, "T must inherit from TaskBase");
        
        creators_[type_name] = [](const std::string& config) -> std::unique_ptr<TaskBase> {
            return std::make_unique<T>(T::from_config(config));
        };
    }
    
    std::unique_ptr<TaskBase> create_task(const std::string& type, const std::string& config) {
        auto it = creators_.find(type);
        if (it != creators_.end()) {
            return it->second(config);
        }
        throw std::runtime_error("Unknown task type: " + type);
    }
    
    bool is_registered(const std::string& type) const {
        return creators_.find(type) != creators_.end();
    }
    
    std::vector<std::string> get_registered_types() const {
        std::vector<std::string> types;
        for (const auto& [type, _] : creators_) {
            types.push_back(type);
        }
        return types;
    }
    
    static TaskFactory& instance() {
        static TaskFactory factory;
        return factory;
    }
    
private:
    TaskFactory() = default;
    std::map<std::string, TaskCreator> creators_;
};

// ============================================================================
// TASK REGISTRY: Automatic registration helper
// ============================================================================

template<typename T>
class TaskRegistrar {
public:
    explicit TaskRegistrar(const std::string& type_name) {
        TaskFactory::instance().register_task<T>(type_name);
    }
};

// Macro for easy task registration
#define REGISTER_TASK(TaskClass, TypeName) \
    static TaskRegistrar<TaskClass> g_##TaskClass##_registrar(TypeName)

// ============================================================================
// ABSTRACT FACTORY: For creating different categories of tasks
// ============================================================================

enum class TaskCategory {
    DataProcessing,
    NetworkOperation,
    FileOperation,
    Computation
};

class AbstractTaskFactory {
public:
    virtual ~AbstractTaskFactory() = default;
    virtual std::unique_ptr<TaskBase> create_task(const std::string& config) = 0;
    virtual TaskCategory get_category() const = 0;
};

class DataProcessingFactory : public AbstractTaskFactory {
public:
    std::unique_ptr<TaskBase> create_task(const std::string& config) override {
        return TaskFactory::instance().create_task("DataProcessing", config);
    }
    
    TaskCategory get_category() const override {
        return TaskCategory::DataProcessing;
    }
};

class NetworkOperationFactory : public AbstractTaskFactory {
public:
    std::unique_ptr<TaskBase> create_task(const std::string& config) override {
        return TaskFactory::instance().create_task("NetworkOperation", config);
    }
    
    TaskCategory get_category() const override {
        return TaskCategory::NetworkOperation;
    }
};

// Factory of factories
class TaskFactoryManager {
public:
    void register_factory(TaskCategory category, std::unique_ptr<AbstractTaskFactory> factory) {
        factories_[category] = std::move(factory);
    }
    
    std::unique_ptr<TaskBase> create_task(TaskCategory category, const std::string& config) {
        auto it = factories_.find(category);
        if (it != factories_.end()) {
            return it->second->create_task(config);
        }
        throw std::runtime_error("No factory registered for category");
    }
    
    static TaskFactoryManager& instance() {
        static TaskFactoryManager manager;
        return manager;
    }
    
private:
    std::map<TaskCategory, std::unique_ptr<AbstractTaskFactory>> factories_;
};

} // namespace dtpf