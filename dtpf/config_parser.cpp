// config_parser.cpp
// Regular expression-based configuration parsing

#include <regex>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <sstream>
#include <fstream>
#include <set>

namespace dtpf {

// ============================================================================
// CONFIGURATION PARSER USING REGULAR EXPRESSIONS
// ============================================================================

class ConfigParser {
public:
    // Configuration structures
    struct TaskConfig {
        std::string type;
        std::map<std::string, std::string> properties;
        int priority = 0;
        std::string node_assignment;
        
        bool has_property(const std::string& key) const {
            return properties.find(key) != properties.end();
        }
        
        std::string get_property(const std::string& key, const std::string& default_value = "") const {
            auto it = properties.find(key);
            return (it != properties.end()) ? it->second : default_value;
        }
    };
    
    struct RoutingRule {
        std::string from_node;
        std::string to_node;
        std::regex condition;
        std::string condition_string;
        
        bool matches(const std::string& data) const {
            return std::regex_search(data, condition);
        }
    };
    
    struct NodeConfig {
        std::string name;
        std::string address;
        int port = 8080;
        int max_workers = 4;
        std::map<std::string, std::string> properties;
    };
    
    struct PipelineConfig {
        std::vector<std::string> stages;
        std::string execution_policy = "sequential";
    };

private:
    // Regex pattern getter functions
    static const std::regex& get_task_pattern() {
        static const std::regex pattern{R"(task\s*:\s*(\w+)\s*\{([^}]*)\})"};
        return pattern;
    }
    
    static const std::regex& get_property_pattern() {
        // Pattern matches: key = "value"
        static const std::regex pattern("(\\w+)\\s*=\\s*\"([^\"]*)\"");
        return pattern;
    }
    
    static const std::regex& get_routing_pattern() {
        static const std::regex pattern{R"(route\s*:\s*(\w+)\s*->\s*(\w+)(?:\s*where\s+(.+))?)"};
        return pattern;
    }
    
    static const std::regex& get_condition_pattern() {
        static const std::regex pattern{R"((\w+)\s*([><=!]+)\s*(\w+))"};
        return pattern;
    }
    
    static const std::regex& get_pipeline_pattern() {
        static const std::regex pattern{R"(pipeline\s*:\s*(.+))"};
        return pattern;
    }
    
    static const std::regex& get_node_pattern() {
        static const std::regex pattern{R"(node\s*:\s*(\w+)\s*\{([^}]*)\})"};
        return pattern;
    }

public:
    // ============================================================================
    // TASK CONFIGURATION PARSING
    // ============================================================================
    
    static std::vector<TaskConfig> parse_tasks(const std::string& config) {
        std::vector<TaskConfig> tasks;
        const auto& task_pattern = get_task_pattern();
        std::sregex_iterator iter(config.begin(), config.end(), task_pattern);
        std::sregex_iterator end;
        
        for (; iter != end; ++iter) {
            TaskConfig task_config;
            task_config.type = (*iter)[1].str();
            
            std::string properties = (*iter)[2].str();
            parse_properties(properties, task_config);
            
            tasks.push_back(std::move(task_config));
        }
        
        return tasks;
    }
    
    // ============================================================================
    // ROUTING CONFIGURATION PARSING
    // ============================================================================
    
    static std::vector<RoutingRule> parse_routing(const std::string& config) {
        std::vector<RoutingRule> rules;
        const auto& routing_pattern = get_routing_pattern();
        std::sregex_iterator iter(config.begin(), config.end(), routing_pattern);
        
        for (; iter != std::sregex_iterator{}; ++iter) {
            RoutingRule rule;
            rule.from_node = (*iter)[1].str();
            rule.to_node = (*iter)[2].str();
            
            if ((*iter)[3].matched) {
                rule.condition_string = (*iter)[3].str();
                rule.condition = std::regex(parse_condition_to_regex(rule.condition_string));
            } else {
                rule.condition_string = ".*";
                rule.condition = std::regex(".*"); // Match all
            }
            
            rules.push_back(std::move(rule));
        }
        
        return rules;
    }
    
    // ============================================================================
    // NODE CONFIGURATION PARSING
    // ============================================================================
    
    static std::vector<NodeConfig> parse_nodes(const std::string& config) {
        std::vector<NodeConfig> nodes;
        const auto& node_pattern = get_node_pattern();
        std::sregex_iterator iter(config.begin(), config.end(), node_pattern);
        
        for (; iter != std::sregex_iterator{}; ++iter) {
            NodeConfig node_config;
            node_config.name = (*iter)[1].str();
            
            std::string properties = (*iter)[2].str();
            parse_node_properties(properties, node_config);
            
            nodes.push_back(std::move(node_config));
        }
        
        return nodes;
    }
    
    // ============================================================================
    // PIPELINE CONFIGURATION PARSING
    // ============================================================================
    
    static std::vector<PipelineConfig> parse_pipelines(const std::string& config) {
        std::vector<PipelineConfig> pipelines;
        const auto& pipeline_pattern = get_pipeline_pattern();
        std::sregex_iterator iter(config.begin(), config.end(), pipeline_pattern);
        
        for (; iter != std::sregex_iterator{}; ++iter) {
            PipelineConfig pipeline_config;
            std::string stages_str = (*iter)[1].str();
            
            // Split stages by '->' 
            std::regex stage_delimiter{R"(\s*->\s*)"};
            std::sregex_token_iterator stage_iter(
                stages_str.begin(), stages_str.end(), stage_delimiter, -1
            );
            
            for (; stage_iter != std::sregex_token_iterator{}; ++stage_iter) {
                std::string stage = stage_iter->str();
                if (!stage.empty()) {
                    pipeline_config.stages.push_back(stage);
                }
            }
            
            pipelines.push_back(std::move(pipeline_config));
        }
        
        return pipelines;
    }
    
    // ============================================================================
    // VALIDATION AND UTILITY METHODS
    // ============================================================================
    
    static bool validate_config(const std::string& config) {
        try {
            auto tasks = parse_tasks(config);
            auto routing = parse_routing(config);
            auto nodes = parse_nodes(config);
            
            // Basic validation
            return !tasks.empty(); // At least one task required
        } catch (const std::exception&) {
            return false;
        }
    }
    
    static std::string extract_section(const std::string& config, const std::string& section_name) {
        std::regex section_pattern{
            R"()" + section_name + R"(\s*\{([^}]*)\})"
        };
        
        std::smatch match;
        if (std::regex_search(config, match, section_pattern)) {
            return match[1].str();
        }
        
        return "";
    }

private:
    // ============================================================================
    // HELPER METHODS
    // ============================================================================
    
    static void parse_properties(const std::string& properties, TaskConfig& task_config) {
        const auto& property_pattern = get_property_pattern();
        std::sregex_iterator prop_iter(properties.begin(), properties.end(), property_pattern);
        
        for (; prop_iter != std::sregex_iterator{}; ++prop_iter) {
            std::string key = (*prop_iter)[1].str();
            std::string value = (*prop_iter)[2].str();
            
            task_config.properties[key] = value;
            
            // Handle special properties
            if (key == "priority") {
                task_config.priority = std::stoi(value);
            } else if (key == "node") {
                task_config.node_assignment = value;
            }
        }
    }
    
    static void parse_node_properties(const std::string& properties, NodeConfig& node_config) {
        const auto& property_pattern = get_property_pattern();
        std::sregex_iterator prop_iter(properties.begin(), properties.end(), property_pattern);
        
        for (; prop_iter != std::sregex_iterator{}; ++prop_iter) {
            std::string key = (*prop_iter)[1].str();
            std::string value = (*prop_iter)[2].str();
            
            node_config.properties[key] = value;
            
            // Handle special properties
            if (key == "address") {
                node_config.address = value;
            } else if (key == "port") {
                node_config.port = std::stoi(value);
            } else if (key == "max_workers") {
                node_config.max_workers = std::stoi(value);
            }
        }
    }
    
    static std::string parse_condition_to_regex(const std::string& condition) {
        const auto& condition_pattern = get_condition_pattern();
        std::smatch match;
        if (std::regex_match(condition, match, condition_pattern)) {
            std::string field = match[1].str();
            std::string op = match[2].str();
            std::string value = match[3].str();
            
            // Convert logical condition to regex pattern
            if (op == "==" || op == "=") {
                return field + R"(\s*:\s*)" + value;
            } else if (op == "!=") {
                return R"((?!)" + field + R"(\s*:\s*)" + value + R"()).*)";
            } else if (op == ">") {
                // For numeric comparisons, this is simplified
                return field + R"(\s*:\s*[0-9]+)"; // Would need more sophisticated parsing
            }
        }
        
        // Default: treat as regex pattern
        return condition;
    }
};

} // namespace dtpf