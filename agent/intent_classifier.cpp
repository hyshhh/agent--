#include "intent_classifier.h"
#include <algorithm>
#include <iostream>

namespace edge_llm_rag
{

    IntentClassifier::IntentClassifier()
    {
        initialize_keywords();
    }

    void IntentClassifier::initialize_keywords()
    {
        emergency_keywords_ = {
            "故障", "警告", "危险", "紧急", "异常", "失灵", "失效", "损坏",
            "发动机故障", "制动故障", "转向故障", "电气故障", "安全气囊", "ABS故障",
            "着火", "碰撞", "泄漏", "冒烟"};

        conversation_keywords_ = {
            "你好", "您好", "谢谢", "再见", "帮忙", "请问",
            "笑话", "故事", "新闻", "天气", "推荐", "建议"};

        skill_keywords_["warning_lights"] = {
            "警告灯", "指示灯", "故障灯", "仪表盘", "亮灯",
            "发动机灯", "ABS灯", "气囊灯", "机油灯", "水温灯"};

        skill_keywords_["maintenance_guide"] = {
            "保养", "维护", "更换", "检查", "清洁", "调整", "润滑",
            "机油", "滤清器", "火花塞", "制动片", "轮胎"};

        skill_keywords_["feature_guide"] = {
            "功能", "使用", "操作", "设置", "调节",
            "自动泊车", "车道保持", "定速巡航", "导航", "空调"};

        skill_keywords_["troubleshooting"] = {
            "排除", "诊断", "检查", "问题", "原因", "解决",
            "无法启动", "异响", "抖动", "漏水", "漏油"};

        skill_keywords_["safety_driving"] = {
            "安全", "驾驶", "注意", "建议", "提醒",
            "高速", "雨天", "夜间", "冬季", "夏季"};

        skill_keywords_["emergency_handling"] = {
            "紧急", "处理", "应对", "措施", "救援",
            "爆胎", "没油", "抛锚", "事故", "故障"};

        skill_keywords_["tech_specs"] = {
            "参数", "规格", "数据", "尺寸", "重量",
            "排量", "功率", "扭矩", "轴距", "容积"};

        skill_keywords_["fuel_economy"] = {
            "油耗", "燃油", "经济", "省油", "加油",
            "续航", "里程", "油箱", "节油"};

        skill_keywords_["maintenance_record"] = {
            "记录", "历史", "保养记录", "维修记录",
            "上次", "下次", "周期", "里程"};

        skill_keywords_["warranty_policy"] = {
            "保修", "质保", "arranty", "条款", "政策",
            "保修期", "保修范围", "免费维修"};

        skill_keywords_["contact_info"] = {
            "联系", "电话", "客服", "救援", "咨询",
            "4S店", "经销商", "服务热线"};

        tool_keywords_["nearby_poi_search"] = {
            "附近", "周边", "哪里有", "最近的",
            "加油站", "停车场", "餐厅", "酒店", "医院"};

        tool_keywords_["route_planning"] = {
            "路线", "导航", "怎么去", "如何到达",
            "规划", "路径", "最快", "最短"};

        tool_keywords_["weather_query"] = {
            "天气", "气温", "下雨", "晴天", "阴天",
            "预报", "今天天气", "明天天气"};
    }

    IntentResult IntentClassifier::classify(const std::string &query)
    {
        IntentResult result;
        result.needs_llm_decision = false;

        IntentType emergency = detect_emergency(query);
        if (emergency == IntentType::EMERGENCY)
        {
            result.intent_type = IntentType::EMERGENCY;
            result.confidence = 0.9f;
            result.reasoning = "检测到紧急关键词";
            return result;
        }

        if (is_conversation(query))
        {
            result.intent_type = IntentType::CONVERSATION;
            result.confidence = 0.8f;
            result.reasoning = "对话/闲聊类型";
            result.needs_llm_decision = true;
            return result;
        }

        std::string matched_tool = match_tool(query);
        if (!matched_tool.empty())
        {
            result.intent_type = IntentType::TOOL_USE;
            result.matched_tool = matched_tool;
            result.confidence = 0.85f;
            result.reasoning = "匹配到工具: " + matched_tool;
            return result;
        }

        std::string matched_skill = match_skill(query);
        if (!matched_skill.empty())
        {
            result.intent_type = IntentType::KNOWLEDGE;
            result.matched_skill = matched_skill;
            result.confidence = 0.8f;
            result.reasoning = "匹配到技能: " + matched_skill;
            return result;
        }

        result.intent_type = IntentType::UNKNOWN;
        result.confidence = 0.3f;
        result.needs_llm_decision = true;
        result.reasoning = "未匹配到明确意图，需要LLM决策";
        return result;
    }

    IntentType IntentClassifier::detect_emergency(const std::string &query)
    {
        for (const auto &keyword : emergency_keywords_)
        {
            if (query.find(keyword) != std::string::npos)
            {
                return IntentType::EMERGENCY;
            }
        }
        return IntentType::UNKNOWN;
    }

    std::string IntentClassifier::match_skill(const std::string &query)
    {
        std::string best_match;
        int max_matches = 0;

        for (const auto &[skill_name, keywords] : skill_keywords_)
        {
            int match_count = 0;
            for (const auto &keyword : keywords)
            {
                if (query.find(keyword) != std::string::npos)
                {
                    match_count++;
                }
            }

            if (match_count > max_matches)
            {
                max_matches = match_count;
                best_match = skill_name;
            }
        }

        return max_matches >= 1 ? best_match : "";
    }

    std::string IntentClassifier::match_tool(const std::string &query)
    {
        for (const auto &[tool_name, keywords] : tool_keywords_)
        {
            for (const auto &keyword : keywords)
            {
                if (query.find(keyword) != std::string::npos)
                {
                    return tool_name;
                }
            }
        }
        return "";
    }

    bool IntentClassifier::is_conversation(const std::string &query)
    {
        for (const auto &keyword : conversation_keywords_)
        {
            if (query.find(keyword) != std::string::npos)
            {
                return true;
            }
        }
        return false;
    }

    float IntentClassifier::calculate_confidence(IntentType intent_type,
                                                  const std::string &query)
    {
        float base_confidence = 0.5f;

        switch (intent_type)
        {
        case IntentType::EMERGENCY:
            base_confidence = 0.9f;
            break;
        case IntentType::KNOWLEDGE:
        case IntentType::TOOL_USE:
            base_confidence = 0.8f;
            break;
        case IntentType::CONVERSATION:
            base_confidence = 0.7f;
            break;
        default:
            base_confidence = 0.3f;
        }

        return base_confidence;
    }

    void IntentClassifier::set_skill_keywords(const std::string &skill_name,
                                               const std::vector<std::string> &keywords)
    {
        skill_keywords_[skill_name] = keywords;
    }

    void IntentClassifier::set_tool_keywords(const std::string &tool_name,
                                              const std::vector<std::string> &keywords)
    {
        tool_keywords_[tool_name] = keywords;
    }

} // namespace edge_llm_rag