#pragma once

#include "base_tool.h"

namespace edge_llm_rag
{

    class VehicleStatusTool : public BaseTool
    {
    public:
        explicit VehicleStatusTool();
        ~VehicleStatusTool() override = default;

        ToolResult execute(const std::unordered_map<std::string, std::string> &params) override;
        std::string get_help() const override;

    private:
        struct VehicleStatus
        {
            std::string status_type;
            std::string value;
            std::string unit;
            std::string warning;
        };

        VehicleStatus simulate_status_query(const std::string &status_type);
    };

} // namespace edge_llm_rag