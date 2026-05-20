#include "agent_engine.h"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace edge_llm_rag
{

    AgentEngine::AgentEngine()
    {
        intent_classifier_ = std::make_unique<IntentClassifier>();
    }

    void AgentEngine::register_skill(SkillPtr skill)
    {
        if (skill)
        {
            skills_[skill->get_name()] = skill;
            intent_classifier_->set_skill_keywords(skill->get_name(),
                                                    skill->get_keywords());
        }
    }

    void AgentEngine::register_tool(ToolPtr tool)
    {
        if (tool)
        {
            tools_[tool->get_name()] = tool;
        }
    }

    void AgentEngine::set_llm_callback(LLMCallback callback)
    {
        llm_callback_ = callback;
    }

    AgentResponse AgentEngine::process(const std::string &query)
    {
        IntentResult intent = intent_classifier_->classify(query);

        std::cout << "[Agent] Intent: " << static_cast<int>(intent.intent_type)
                  << ", Confidence: " << intent.confidence
                  << ", Reasoning: " << intent.reasoning << std::endl;

        if (!intent.needs_llm_decision && intent.confidence >= 0.7f)
        {
            return process_with_fast_path(query, intent);
        }

        return process_with_llm(query);
    }

    AgentResponse AgentEngine::process_with_fast_path(const std::string &query,
                                                       const IntentResult &intent)
    {
        AgentResponse response;
        response.used_llm = false;
        response.confidence = intent.confidence;

        switch (intent.intent_type)
        {
        case IntentType::EMERGENCY:
        {
            if (!intent.matched_skill.empty())
            {
                SkillResult skill_result = execute_skill(intent.matched_skill, query);
                response.content = skill_result.content;
                response.skills_used.push_back(intent.matched_skill);
            }
            else
            {
                SkillResult skill_result = execute_skill("emergency_handling", query);
                response.content = skill_result.content;
                response.skills_used.push_back("emergency_handling");
            }
            break;
        }

        case IntentType::KNOWLEDGE:
        {
            if (!intent.matched_skill.empty())
            {
                SkillResult skill_result = execute_skill(intent.matched_skill, query);
                response.content = skill_result.content;
                response.skills_used.push_back(intent.matched_skill);
            }
            break;
        }

        case IntentType::TOOL_USE:
        {
            if (!intent.matched_tool.empty())
            {
                std::unordered_map<std::string, std::string> params;
                params["query"] = query;
                ToolResult tool_result = execute_tool(intent.matched_tool, params);
                response.content = tool_result.content;
                response.tools_used.push_back(intent.matched_tool);
            }
            break;
        }

        case IntentType::CONVERSATION:
        {
            response.used_llm = true;
            if (llm_callback_)
            {
                response.content = llm_callback_(query);
            }
            break;
        }

        default:
        {
            response.used_llm = true;
            if (llm_callback_)
            {
                response.content = llm_callback_(query);
            }
            break;
        }
        }

        return response;
    }

    AgentResponse AgentEngine::process_with_llm(const std::string &query)
    {
        AgentResponse response;
        response.used_llm = true;
        response.confidence = 0.5f;

        if (!llm_callback_)
        {
            response.content = "抱歉，我暂时无法处理您的请求。";
            return response;
        }

        std::string agent_prompt = build_agent_prompt(query);
        std::string llm_output = llm_callback_(agent_prompt);

        std::string tool_call = parse_tool_call(llm_output);

        if (!tool_call.empty())
        {
            std::string tool_name;
            std::unordered_map<std::string, std::string> params;

            size_t paren_pos = tool_call.find('(');
            if (paren_pos != std::string::npos)
            {
                tool_name = tool_call.substr(0, paren_pos);
                std::string params_str = tool_call.substr(paren_pos + 1);
                size_t end_pos = params_str.find(')');
                if (end_pos != std::string::npos)
                {
                    params_str = params_str.substr(0, end_pos);
                }
                params = parse_json_params(params_str);
            }
            else
            {
                tool_name = tool_call;
            }

            if (tools_.find(tool_name) != tools_.end())
            {
                ToolResult tool_result = execute_tool(tool_name, params);
                response.content = tool_result.content;
                response.tools_used.push_back(tool_name);
            }
            else if (skills_.find(tool_name) != skills_.end())
            {
                SkillResult skill_result = execute_skill(tool_name, query);
                response.content = skill_result.content;
                response.skills_used.push_back(tool_name);
            }
            else
            {
                response.content = llm_output;
            }
        }
        else
        {
            response.content = llm_output;
        }

        return response;
    }

    std::string AgentEngine::build_agent_prompt(const std::string &query)
    {
        std::string prompt = "你是一个智能座舱助手。你可以使用以下工具：\n\n";

        prompt += "可用工具：\n";
        for (const auto &[name, tool] : tools_)
        {
            if (tool->is_active())
            {
                prompt += "- " + tool->get_schema() + "\n";
            }
        }

        prompt += "\n可用知识技能：\n";
        for (const auto &[name, skill] : skills_)
        {
            if (skill->is_active())
            {
                prompt += "- " + skill->get_name() + ": " + skill->get_description() + "\n";
            }
        }

        prompt += "\n请根据用户问题，决定是否需要调用工具或技能。\n";
        prompt += "如果需要，请输出格式：{\"tool\": \"工具名\", \"parameters\": {...}}\n";
        prompt += "如果不需要，请直接回答。\n\n";
        prompt += "用户问题：" + query;

        return prompt;
    }

    std::string AgentEngine::parse_tool_call(const std::string &llm_output)
    {
        size_t json_start = llm_output.find('{');
        size_t json_end = llm_output.find('}');

        if (json_start != std::string::npos && json_end != std::string::npos)
        {
            std::string json_str = llm_output.substr(json_start, json_end - json_start + 1);

            size_t tool_pos = json_str.find("\"tool\"");
            if (tool_pos != std::string::npos)
            {
                size_t colon_pos = json_str.find(':', tool_pos);
                size_t quote_start = json_str.find('"', colon_pos + 1);
                size_t quote_end = json_str.find('"', quote_start + 1);

                if (quote_start != std::string::npos && quote_end != std::string::npos)
                {
                    return json_str.substr(quote_start + 1, quote_end - quote_start - 1);
                }
            }
        }

        return "";
    }

    ToolResult AgentEngine::execute_tool(const std::string &tool_name,
                                          const std::unordered_map<std::string, std::string> &params)
    {
        auto it = tools_.find(tool_name);
        if (it == tools_.end())
        {
            ToolResult result;
            result.success = false;
            result.tool_name = tool_name;
            result.error_message = "工具不存在: " + tool_name;
            return result;
        }

        return it->second->execute(params);
    }

    SkillResult AgentEngine::execute_skill(const std::string &skill_name,
                                            const std::string &query)
    {
        auto it = skills_.find(skill_name);
        if (it == skills_.end())
        {
            SkillResult result;
            result.success = false;
            result.skill_name = skill_name;
            result.content = "技能不存在: " + skill_name;
            return result;
        }

        return it->second->execute(query);
    }

    std::unordered_map<std::string, std::string> AgentEngine::parse_json_params(const std::string &json_str)
    {
        std::unordered_map<std::string, std::string> params;

        std::string clean_str = json_str;
        clean_str.erase(std::remove_if(clean_str.begin(), clean_str.end(),
                                        [](char c)
                                        { return c == '{' || c == '}' || c == '"'; }),
                        clean_str.end());

        std::istringstream iss(clean_str);
        std::string token;

        while (std::getline(iss, token, ','))
        {
            size_t colon_pos = token.find(':');
            if (colon_pos != std::string::npos)
            {
                std::string key = token.substr(0, colon_pos);
                std::string value = token.substr(colon_pos + 1);

                key.erase(std::remove_if(key.begin(), key.end(), ::isspace), key.end());
                value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());

                params[key] = value;
            }
        }

        return params;
    }

    std::string AgentEngine::get_available_skills_schema() const
    {
        std::string schema;
        for (const auto &[name, skill] : skills_)
        {
            if (skill->is_active())
            {
                schema += skill->get_name() + " - " + skill->get_description() + "\n";
            }
        }
        return schema;
    }

    std::string AgentEngine::get_available_tools_schema() const
    {
        std::string schema;
        for (const auto &[name, tool] : tools_)
        {
            if (tool->is_active())
            {
                schema += tool->get_schema() + "\n";
            }
        }
        return schema;
    }

    SkillPtr AgentEngine::get_skill(const std::string &name)
    {
        auto it = skills_.find(name);
        return it != skills_.end() ? it->second : nullptr;
    }

    ToolPtr AgentEngine::get_tool(const std::string &name)
    {
        auto it = tools_.find(name);
        return it != tools_.end() ? it->second : nullptr;
    }

} // namespace edge_llm_rag