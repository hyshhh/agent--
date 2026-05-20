#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include "base_skill.h"

namespace edge_llm_rag
{

    class SkillRegistry
    {
    public:
        static SkillRegistry &instance();

        void register_skill(SkillPtr skill);
        SkillPtr get_skill(const std::string &name);
        std::vector<SkillPtr> get_all_skills();
        std::vector<std::string> get_skill_names();

        void initialize_default_skills();

    private:
        SkillRegistry() = default;
        std::unordered_map<std::string, SkillPtr> skills_;
    };

} // namespace edge_llm_rag