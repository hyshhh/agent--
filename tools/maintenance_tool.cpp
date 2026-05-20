#include "maintenance_tool.h"
#include <iostream>
#include <sstream>

namespace edge_llm_rag
{

    MaintenanceTool::MaintenanceTool()
        : BaseTool("maintenance_reminder", "提供车辆保养提醒和建议")
    {
        add_parameter({"service_type", "string", "保养类型（机油/刹车/轮胎/空调/全面）", true, ""});
    }

    ToolResult MaintenanceTool::execute(const std::unordered_map<std::string, std::string> &params)
    {
        ToolResult result;
        result.tool_name = name_;
        result.success = false;

        auto it = params.find("service_type");
        if (it == params.end())
        {
            result.error_message = "缺少必要参数: service_type";
            return result;
        }

        std::string service_type = it->second;
        MaintenanceInfo info = simulate_maintenance_query(service_type);

        std::ostringstream oss;
        oss << "保养提醒 - " << info.service_type << "：\n\n";
        oss << "保养周期: " << info.interval << "\n";
        oss << "上次保养: " << info.last_service << "\n";
        oss << "下次保养: " << info.next_service << "\n";
        oss << "预计费用: " << info.estimated_cost << "\n\n";
        oss << "保养项目：\n";

        for (const auto &item : info.items)
        {
            oss << "- " << item << "\n";
        }

        result.content = oss.str();
        result.success = true;

        return result;
    }

    std::string MaintenanceTool::get_help() const
    {
        return description_ + "\n"
               "参数:\n"
               "  service_type (必需): 保养类型\n"
               "示例: maintenance_reminder(service_type=机油)";
    }

    MaintenanceTool::MaintenanceInfo MaintenanceTool::simulate_maintenance_query(const std::string &service_type)
    {
        MaintenanceInfo info;
        info.service_type = service_type;

        if (service_type == "机油" || service_type == "oil")
        {
            info.interval = "每5000公里或6个月";
            info.last_service = "2024-01-15 (20000公里)";
            info.next_service = "2024-04-15 (25000公里)";
            info.estimated_cost = "300-500元";
            info.items = {"更换机油", "更换机油滤清器", "检查机油液位"};
        }
        else if (service_type == "刹车" || service_type == "brake")
        {
            info.interval = "每20000公里或12个月";
            info.last_service = "2023-10-20 (15000公里)";
            info.next_service = "2024-10-20 (35000公里)";
            info.estimated_cost = "200-400元";
            info.items = {"检查刹车片厚度", "检查刹车盘", "更换刹车液", "检查刹车管路"};
        }
        else if (service_type == "轮胎" || service_type == "tire")
        {
            info.interval = "每10000公里或6个月";
            info.last_service = "2024-02-01 (22000公里)";
            info.next_service = "2024-05-01 (32000公里)";
            info.estimated_cost = "100-200元";
            info.items = {"轮胎换位", "检查胎压", "检查轮胎磨损", "轮胎动平衡"};
        }
        else if (service_type == "空调" || service_type == "ac")
        {
            info.interval = "每12个月";
            info.last_service = "2023-06-15";
            info.next_service = "2024-06-15";
            info.estimated_cost = "150-300元";
            info.items = {"更换空调滤清器", "检查制冷剂", "清洁空调系统"};
        }
        else
        {
            info.interval = "每10000公里或6个月";
            info.last_service = "2024-01-15 (20000公里)";
            info.next_service = "2024-04-15 (25000公里)";
            info.estimated_cost = "800-1500元";
            info.items = {"更换机油", "更换机油滤清器", "更换空气滤清器",
                          "更换空调滤清器", "检查刹车系统", "检查轮胎",
                          "检查灯光", "检查电瓶", "检查冷却液"};
        }

        return info;
    }

} // namespace edge_llm_rag