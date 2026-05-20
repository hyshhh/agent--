#pragma once

#include "base_tool.h"

namespace edge_llm_rag
{

    class EmergencyGuideTool : public BaseTool
    {
    public:
        explicit EmergencyGuideTool();
        ~EmergencyGuideTool() override = default;

        ToolResult execute(const std::unordered_map<std::string, std::string> &params) override;
        std::string get_help() const override;

    private:
        struct EmergencyGuide
        {
            std::string emergency_type;
            std::string severity;
            std::vector<std::string> steps;
            std::string warning;
        };

        EmergencyGuide simulate_emergency_guide(const std::string &emergency_type);
    };

} // namespace edge_llm_rag