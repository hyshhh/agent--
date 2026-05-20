#pragma once

#include "base_tool.h"

namespace edge_llm_rag
{

    class MaintenanceTool : public BaseTool
    {
    public:
        explicit MaintenanceTool();
        ~MaintenanceTool() override = default;

        ToolResult execute(const std::unordered_map<std::string, std::string> &params) override;
        std::string get_help() const override;

    private:
        struct MaintenanceInfo
        {
            std::string service_type;
            std::string interval;
            std::string last_service;
            std::string next_service;
            std::string estimated_cost;
            std::vector<std::string> items;
        };

        MaintenanceInfo simulate_maintenance_query(const std::string &service_type);
    };

} // namespace edge_llm_rag