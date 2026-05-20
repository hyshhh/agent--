#include "vehicle_status_tool.h"
#include <iostream>
#include <sstream>

namespace edge_llm_rag
{

    VehicleStatusTool::VehicleStatusTool()
        : BaseTool("get_vehicle_status", "获取车辆当前状态信息")
    {
        add_parameter({"status_type", "string", "状态类型（油量/水温/胎压/里程/电量）", true, ""});
    }

    ToolResult VehicleStatusTool::execute(const std::unordered_map<std::string, std::string> &params)
    {
        ToolResult result;
        result.tool_name = name_;
        result.success = false;

        auto it = params.find("status_type");
        if (it == params.end())
        {
            result.error_message = "缺少必要参数: status_type";
            return result;
        }

        std::string status_type = it->second;
        VehicleStatus status = simulate_status_query(status_type);

        std::ostringstream oss;
        oss << "车辆状态 - " << status.status_type << "：\n\n";
        oss << "当前值: " << status.value << " " << status.unit << "\n";

        if (!status.warning.empty())
        {
            oss << "警告: " << status.warning << "\n";
        }

        result.content = oss.str();
        result.success = true;

        return result;
    }

    std::string VehicleStatusTool::get_help() const
    {
        return description_ + "\n"
               "参数:\n"
               "  status_type (必需): 状态类型\n"
               "示例: get_vehicle_status(status_type=油量)";
    }

    VehicleStatusTool::VehicleStatus VehicleStatusTool::simulate_status_query(const std::string &status_type)
    {
        VehicleStatus status;
        status.status_type = status_type;

        if (status_type == "油量" || status_type == "fuel")
        {
            status.value = "65";
            status.unit = "%";
            status.warning = "";
        }
        else if (status_type == "水温" || status_type == "temperature")
        {
            status.value = "90";
            status.unit = "°C";
            status.warning = "";
        }
        else if (status_type == "胎压" || status_type == "tire_pressure")
        {
            status.value = "2.4";
            status.unit = "bar";
            status.warning = "左后轮胎压偏低";
        }
        else if (status_type == "里程" || status_type == "mileage")
        {
            status.value = "25680";
            status.unit = "公里";
            status.warning = "";
        }
        else if (status_type == "电量" || status_type == "battery")
        {
            status.value = "85";
            status.unit = "%";
            status.warning = "";
        }
        else
        {
            status.value = "正常";
            status.unit = "";
            status.warning = "";
        }

        return status;
    }

} // namespace edge_llm_rag