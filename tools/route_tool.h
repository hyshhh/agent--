#pragma once

#include "base_tool.h"

namespace edge_llm_rag
{

    class RouteTool : public BaseTool
    {
    public:
        explicit RouteTool();
        ~RouteTool() override = default;

        ToolResult execute(const std::unordered_map<std::string, std::string> &params) override;
        std::string get_help() const override;

    private:
        struct RouteInfo
        {
            std::string destination;
            std::string distance;
            std::string duration;
            std::string route_type;
            std::string description;
        };

        RouteInfo simulate_route_planning(const std::string &destination);
    };

} // namespace edge_llm_rag