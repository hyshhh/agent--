#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace edge_llm_rag
{

    enum class IntentType
    {
        EMERGENCY,      // 紧急情况
        KNOWLEDGE,      // 知识查询
        TOOL_USE,       // 工具调用
        CONVERSATION,   // 对话/闲聊
        UNKNOWN         // 未知
    };

    struct IntentResult
    {
        IntentType intent_type;
        float confidence;
        std::string matched_skill;
        std::string matched_tool;
        bool needs_llm_decision;
        std::string reasoning;
    };

    class IntentClassifier
    {
    public:
        explicit IntentClassifier();

        IntentResult classify(const std::string &query);

        void set_skill_keywords(const std::string &skill_name,
                                const std::vector<std::string> &keywords);

        void set_tool_keywords(const std::string &tool_name,
                               const std::vector<std::string> &keywords);

    private:
        std::unordered_map<std::string, std::vector<std::string>> skill_keywords_;
        std::unordered_map<std::string, std::vector<std::string>> tool_keywords_;
        std::vector<std::string> emergency_keywords_;
        std::vector<std::string> conversation_keywords_;

        void initialize_keywords();

        IntentType detect_emergency(const std::string &query);

        std::string match_skill(const std::string &query);

        std::string match_tool(const std::string &query);

        bool is_conversation(const std::string &query);

        float calculate_confidence(IntentType intent_type,
                                   const std::string &query);
    };

} // namespace edge_llm_rag