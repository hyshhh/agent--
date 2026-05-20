#include "weather_tool.h"
#include <iostream>
#include <sstream>

namespace edge_llm_rag
{

    WeatherTool::WeatherTool()
        : BaseTool("weather_query", "查询指定位置的天气信息")
    {
        add_parameter({"location", "string", "查询位置（城市名）", true, ""});
    }

    ToolResult WeatherTool::execute(const std::unordered_map<std::string, std::string> &params)
    {
        ToolResult result;
        result.tool_name = name_;
        result.success = false;

        auto it = params.find("location");
        if (it == params.end())
        {
            result.error_message = "缺少必要参数: location";
            return result;
        }

        std::string location = it->second;
        WeatherInfo weather = simulate_weather_query(location);

        std::ostringstream oss;
        oss << weather.location << "天气预报：\n\n";
        oss << "日期: " << weather.date << "\n";
        oss << "天气: " << weather.condition << "\n";
        oss << "温度: " << weather.temperature_low << "°C ~ " << weather.temperature_high << "°C\n";
        oss << "风力: " << weather.wind << "\n";
        oss << "湿度: " << weather.humidity << "\n";

        result.content = oss.str();
        result.success = true;

        return result;
    }

    std::string WeatherTool::get_help() const
    {
        return description_ + "\n"
               "参数:\n"
               "  location (必需): 城市名称\n"
               "示例: weather_query(location=北京)";
    }

    WeatherTool::WeatherInfo WeatherTool::simulate_weather_query(const std::string &location)
    {
        WeatherInfo weather;
        weather.location = location;
        weather.date = "今天";

        if (location.find("北京") != std::string::npos)
        {
            weather.condition = "晴转多云";
            weather.temperature_high = 28;
            weather.temperature_low = 18;
            weather.wind = "北风3-4级";
            weather.humidity = "45%";
        }
        else if (location.find("上海") != std::string::npos)
        {
            weather.condition = "多云";
            weather.temperature_high = 26;
            weather.temperature_low = 20;
            weather.wind = "东风2-3级";
            weather.humidity = "65%";
        }
        else if (location.find("广州") != std::string::npos)
        {
            weather.condition = "雷阵雨";
            weather.temperature_high = 32;
            weather.temperature_low = 25;
            weather.wind = "南风2级";
            weather.humidity = "80%";
        }
        else
        {
            weather.condition = "晴";
            weather.temperature_high = 25;
            weather.temperature_low = 15;
            weather.wind = "微风";
            weather.humidity = "50%";
        }

        return weather;
    }

} // namespace edge_llm_rag