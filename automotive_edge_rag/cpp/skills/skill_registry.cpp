#include "skill_registry.h"
#include <iostream>

namespace edge_llm_rag
{

    SkillRegistry &SkillRegistry::instance()
    {
        static SkillRegistry registry;
        return registry;
    }

    void SkillRegistry::register_skill(SkillPtr skill)
    {
        if (skill)
        {
            skills_[skill->get_name()] = skill;
            std::cout << "[SkillRegistry] Registered skill: " << skill->get_name() << std::endl;
        }
    }

    SkillPtr SkillRegistry::get_skill(const std::string &name)
    {
        auto it = skills_.find(name);
        return it != skills_.end() ? it->second : nullptr;
    }

    std::vector<SkillPtr> SkillRegistry::get_all_skills()
    {
        std::vector<SkillPtr> result;
        for (const auto &[name, skill] : skills_)
        {
            result.push_back(skill);
        }
        return result;
    }

    std::vector<std::string> SkillRegistry::get_skill_names()
    {
        std::vector<std::string> names;
        for (const auto &[name, skill] : skills_)
        {
            names.push_back(name);
        }
        return names;
    }

    void SkillRegistry::initialize_default_skills()
    {
        std::cout << "[SkillRegistry] Initializing default skills..." << std::endl;
    }

} // namespace edge_llm_rag