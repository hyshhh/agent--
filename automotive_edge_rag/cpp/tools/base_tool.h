#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace edge_llm_rag
{

    struct ToolParameter
    {
        std::string name;
        std::string type;
        std::string description;
        bool required;
        std::string default_value;
    };

    struct ToolResult
    {
        bool success;
        std::string content;
        std::string tool_name;
        std::string error_message;
    };

    class BaseTool
    {
    public:
        explicit BaseTool(const std::string &name,
                          const std::string &description)
            : name_(name), description_(description), is_active_(true) {}

        virtual ~BaseTool() = default;

        const std::string &get_name() const { return name_; }
        const std::string &get_description() const { return description_; }
        bool is_active() const { return is_active_; }
        void set_active(bool active) { is_active_ = active; }

        void add_parameter(const ToolParameter &param)
        {
            parameters_.push_back(param);
        }

        const std::vector<ToolParameter> &get_parameters() const { return parameters_; }

        std::string get_schema() const
        {
            std::string schema = name_ + "(";
            for (size_t i = 0; i < parameters_.size(); ++i)
            {
                if (i > 0)
                    schema += ", ";
                schema += parameters_[i].name;
                if (parameters_[i].required)
                    schema += "*";
            }
            schema += ") - " + description_;
            return schema;
        }

        virtual ToolResult execute(const std::unordered_map<std::string, std::string> &params) = 0;

        virtual std::string get_help() const = 0;

    protected:
        std::string name_;
        std::string description_;
        std::vector<ToolParameter> parameters_;
        bool is_active_;
    };

    using ToolPtr = std::shared_ptr<BaseTool>;

} // namespace edge_llm_rag