#include "route_tool.h"
#include <iostream>
#include <sstream>

namespace edge_llm_rag
{

    RouteTool::RouteTool()
        : BaseTool("route_planning", "规划到目的地的路线")
    {
        add_parameter({"destination", "string", "目的地名称", true, ""});
        add_parameter({"origin", "string", "出发地点（可选，默认当前位置）", false, "当前"});
    }

    ToolResult RouteTool::execute(const std::unordered_map<std::string, std::string> &params)
    {
        ToolResult result;
        result.tool_name = name_;
        result.success = false;

        auto it = params.find("destination");
        if (it == params.end())
        {
            result.error_message = "缺少必要参数: destination";
            return result;
        }

        std::string destination = it->second;
        RouteInfo route = simulate_route_planning(destination);

        std::ostringstream oss;
        oss << "路线规划 - " << route.destination << "：\n\n";
        oss << "距离: " << route.distance << "\n";
        oss << "预计时间: " << route.duration << "\n";
        oss << "路线类型: " << route.route_type << "\n";
        oss << "路线描述: " << route.description << "\n";

        result.content = oss.str();
        result.success = true;

        return result;
    }

    std::string RouteTool::get_help() const
    {
        return description_ + "\n"
               "参数:\n"
               "  destination (必需): 目的地名称\n"
               "  origin (可选): 出发地点，默认当前位置\n"
               "示例: route_planning(destination=北京首都机场)";
    }

    RouteTool::RouteInfo RouteTool::simulate_route_planning(const std::string &destination)
    {
        RouteInfo route;
        route.destination = destination;

        if (destination.find("机场") != std::string::npos)
        {
            route.distance = "35公里";
            route.duration = "约40分钟";
            route.route_type = "高速优先";
            route.description = "从当前位置出发，经三环路进入机场高速，到达目的地";
        }
        else if (destination.find("火车站") != std::string::npos)
        {
            route.distance = "15公里";
            route.duration = "约30分钟";
            route.route_type = "城市道路";
            route.description = "从当前位置出发，经长安街西行到达北京站";
        }
        else if (destination.find("医院") != std::string::npos)
        {
            route.distance = "8公里";
            route.duration = "约20分钟";
            route.route_type = "城市道路";
            route.description = "从当前位置出发，经东三环到达朝阳医院";
        }
        else
        {
            route.distance = "10公里";
            route.duration = "约25分钟";
            route.route_type = "推荐路线";
            route.description = "从当前位置出发，经主干道到达目的地";
        }

        return route;
    }

} // namespace edge_llm_rag