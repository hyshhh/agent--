#include "emergency_tool.h"
#include <iostream>
#include <sstream>

namespace edge_llm_rag
{

    EmergencyGuideTool::EmergencyGuideTool()
        : BaseTool("emergency_guide", "提供紧急情况处理指南")
    {
        add_parameter({"emergency_type", "string", "紧急情况类型（爆胎/没油/抛锚/事故/故障）", true, ""});
    }

    ToolResult EmergencyGuideTool::execute(const std::unordered_map<std::string, std::string> &params)
    {
        ToolResult result;
        result.tool_name = name_;
        result.success = false;

        auto it = params.find("emergency_type");
        if (it == params.end())
        {
            result.error_message = "缺少必要参数: emergency_type";
            return result;
        }

        std::string emergency_type = it->second;
        EmergencyGuide guide = simulate_emergency_guide(emergency_type);

        std::ostringstream oss;
        oss << "紧急处理指南 - " << guide.emergency_type << "：\n\n";
        oss << "严重程度: " << guide.severity << "\n\n";
        oss << "处理步骤：\n";

        for (size_t i = 0; i < guide.steps.size(); ++i)
        {
            oss << (i + 1) << ". " << guide.steps[i] << "\n";
        }

        if (!guide.warning.empty())
        {
            oss << "\n注意: " << guide.warning << "\n";
        }

        result.content = oss.str();
        result.success = true;

        return result;
    }

    std::string EmergencyGuideTool::get_help() const
    {
        return description_ + "\n"
               "参数:\n"
               "  emergency_type (必需): 紧急情况类型\n"
               "示例: emergency_guide(emergency_type=爆胎)";
    }

    EmergencyGuideTool::EmergencyGuide EmergencyGuideTool::simulate_emergency_guide(const std::string &emergency_type)
    {
        EmergencyGuide guide;
        guide.emergency_type = emergency_type;

        if (emergency_type == "爆胎" || emergency_type == "tire_blowout")
        {
            guide.severity = "高";
            guide.steps = {
                "紧握方向盘，保持车辆直线行驶",
                "轻踩刹车，逐渐减速",
                "打开危险警示灯",
                "将车辆慢慢停到路边安全位置",
                "在车后方150米处放置三角警示牌",
                "更换备胎或拨打救援电话"};
            guide.warning = "切勿急刹车或急打方向";
        }
        else if (emergency_type == "没油" || emergency_type == "out_of_fuel")
        {
            guide.severity = "中";
            guide.steps = {
                "打开危险警示灯",
                "将车辆慢慢停到路边安全位置",
                "联系救援服务或就近加油站",
                "在车后方放置三角警示牌"};
            guide.warning = "不要尝试推车行驶";
        }
        else if (emergency_type == "抛锚" || emergency_type == "breakdown")
        {
            guide.severity = "中";
            guide.steps = {
                "打开危险警示灯",
                "将车辆慢慢停到路边安全位置",
                "尝试检查明显故障原因",
                "拨打救援电话",
                "在车后方放置三角警示牌"};
            guide.warning = "不要在车流中逗留";
        }
        else if (emergency_type == "事故" || emergency_type == "accident")
        {
            guide.severity = "高";
            guide.steps = {
                "立即停车，打开危险警示灯",
                "检查是否有人受伤",
                "如有伤者，立即拨打120急救电话",
                "拨打122交通事故报警电话",
                "在车后方放置三角警示牌",
                "拍照记录现场",
                "等待交警处理"};
            guide.warning = "不要移动伤者，除非有 immediate danger";
        }
        else
        {
            guide.severity = "中";
            guide.steps = {
                "打开危险警示灯",
                "将车辆慢慢停到路边安全位置",
                "检查故障原因",
                "联系救援服务"};
            guide.warning = "";
        }

        return guide;
    }

} // namespace edge_llm_rag