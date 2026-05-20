#pragma once

#include "base_tool.h"

namespace edge_llm_rag
{

    class NearbyPOITool : public BaseTool
    {
    public:
        explicit NearbyPOITool();
        ~NearbyPOITool() override = default;

        ToolResult execute(const std::unordered_map<std::string, std::string> &params) override;
        std::string get_help() const override;

    private:
        struct POIInfo
        {
            std::string name;
            std::string type;
            std::string address;
            float distance;
        };

        std::vector<POIInfo> simulate_poi_search(const std::string &poi_type);
    };

} // namespace edge_llm_rag