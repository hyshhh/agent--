#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace edge_llm_rag
{

    struct SkillResult
    {
        bool success;
        std::string content;
        std::string skill_name;
        float relevance_score;
        std::vector<std::string> sources;
    };

    class BaseSkill
    {
    public:
        explicit BaseSkill(const std::string &name,
                           const std::string &description,
                           const std::string &domain)
            : name_(name), description_(description), domain_(domain), is_active_(true) {}

        virtual ~BaseSkill() = default;

        const std::string &get_name() const { return name_; }
        const std::string &get_description() const { return description_; }
        const std::string &get_domain() const { return domain_; }
        bool is_active() const { return is_active_; }
        void set_active(bool active) { is_active_ = active; }

        void set_keywords(const std::vector<std::string> &keywords)
        {
            keywords_ = keywords;
        }

        const std::vector<std::string> &get_keywords() const { return keywords_; }

        bool matches_query(const std::string &query) const
        {
            for (const auto &keyword : keywords_)
            {
                if (query.find(keyword) != std::string::npos)
                {
                    return true;
                }
            }
            return false;
        }

        virtual SkillResult execute(const std::string &query,
                                    const std::unordered_map<std::string, std::string> &params = {}) = 0;

        virtual std::string get_help() const = 0;

    protected:
        std::string name_;
        std::string description_;
        std::string domain_;
        std::vector<std::string> keywords_;
        bool is_active_;
    };

    using SkillPtr = std::shared_ptr<BaseSkill>;

} // namespace edge_llm_rag