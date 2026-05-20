#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>

#include "intent_classifier.h"
#include "../skills/base_skill.h"
#include "../tools/base_tool.h"

namespace edge_llm_rag
{

    struct AgentResponse
    {
        std::string content;
        std::vector<std::string> tools_used;
        std::vector<std::string> skills_used;
        float confidence;
        bool used_llm;
    };

    using LLMCallback = std::function<std::string(const std::string &)>;

    class AgentEngine
    {
    public:
        explicit AgentEngine();
        ~AgentEngine() = default;

        void register_skill(SkillPtr skill);
        void register_tool(ToolPtr tool);

        void set_llm_callback(LLMCallback callback);

        AgentResponse process(const std::string &query);

        std::string get_available_skills_schema() const;
        std::string get_available_tools_schema() const;

        SkillPtr get_skill(const std::string &name);
        ToolPtr get_tool(const std::string &name);

    private:
        std::unique_ptr<IntentClassifier> intent_classifier_;
        std::unordered_map<std::string, SkillPtr> skills_;
        std::unordered_map<std::string, ToolPtr> tools_;
        LLMCallback llm_callback_;

        AgentResponse process_with_fast_path(const std::string &query,
                                              const IntentResult &intent);

        AgentResponse process_with_llm(const std::string &query);

        std::string build_agent_prompt(const std::string &query);

        std::string parse_tool_call(const std::string &llm_output);

        ToolResult execute_tool(const std::string &tool_name,
                                const std::unordered_map<std::string, std::string> &params);

        SkillResult execute_skill(const std::string &skill_name,
                                  const std::string &query);

        std::unordered_map<std::string, std::string> parse_json_params(const std::string &json_str);
    };

} // namespace edge_llm_rag