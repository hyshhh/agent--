#pragma once

#include "base_tool.h"

namespace edge_llm_rag
{

    class WeatherTool : public BaseTool
    {
    public:
        explicit WeatherTool();
        ~WeatherTool() override = default;

        ToolResult execute(const std::unordered_map<std::string, std::string> &params) override;
        std::string get_help() const override;

    private:
        struct WeatherInfo
        {
            std::string location;
            std::string date;
            std::string condition;
            int temperature_high;
            int temperature_low;
            std::string wind;
            std::string humidity;
        };

        WeatherInfo simulate_weather_query(const std::string &location);
    };

} // namespace edge_llm_rag