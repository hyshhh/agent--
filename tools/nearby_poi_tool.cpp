#include "nearby_poi_tool.h"
#include <iostream>
#include <sstream>

namespace edge_llm_rag
{

    NearbyPOITool::NearbyPOITool()
        : BaseTool("nearby_poi_search", "搜索附近的兴趣点（加油站、停车场、餐厅等）")
    {
        add_parameter({"poi_type", "string", "兴趣点类型（加油站/停车场/餐厅/酒店/医院）", true, ""});
        add_parameter({"location", "string", "当前位置（可选，默认当前位置）", false, "当前"});
    }

    ToolResult NearbyPOITool::execute(const std::unordered_map<std::string, std::string> &params)
    {
        ToolResult result;
        result.tool_name = name_;
        result.success = false;

        auto it = params.find("poi_type");
        if (it == params.end())
        {
            result.error_message = "缺少必要参数: poi_type";
            return result;
        }

        std::string poi_type = it->second;
        std::vector<POIInfo> pois = simulate_poi_search(poi_type);

        if (pois.empty())
        {
            result.content = "未找到附近的" + poi_type;
            return result;
        }

        std::ostringstream oss;
        oss << "找到以下附近的" << poi_type << "：\n\n";

        for (size_t i = 0; i < pois.size(); ++i)
        {
            oss << (i + 1) << ". " << pois[i].name << "\n";
            oss << "   地址: " << pois[i].address << "\n";
            oss << "   距离: " << pois[i].distance << "公里\n\n";
        }

        result.content = oss.str();
        result.success = true;

        return result;
    }

    std::string NearbyPOITool::get_help() const
    {
        return description_ + "\n"
               "参数:\n"
               "  poi_type (必需): 兴趣点类型\n"
               "  location (可选): 当前位置\n"
               "示例: nearby_poi_search(poi_type=加油站)";
    }

    std::vector<NearbyPOITool::POIInfo> NearbyPOITool::simulate_poi_search(const std::string &poi_type)
    {
        std::vector<POIInfo> pois;

        if (poi_type == "加油站" || poi_type == "gas station")
        {
            pois.push_back({"中国石化加油站", "加油站", "朝阳区建国路88号", 0.5f});
            pois.push_back({"中国石油加油站", "加油站", "朝阳区东三环中路12号", 1.2f});
            pois.push_back({"壳牌加油站", "加油站", "朝阳区光华路9号", 2.0f});
        }
        else if (poi_type == "停车场" || poi_type == "parking")
        {
            pois.push_back({"万达广场停车场", "停车场", "朝阳区建国路93号", 0.3f});
            pois.push_back({"国贸停车场", "停车场", "朝阳区建国门外大街1号", 0.8f});
        }
        else if (poi_type == "餐厅" || poi_type == "restaurant")
        {
            pois.push_back({"海底捞火锅", "餐厅", "朝阳区建国路89号", 0.4f});
            pois.push_back({"麦当劳", "餐厅", "朝阳区东三环中路15号", 0.6f});
            pois.push_back({"肯德基", "餐厅", "朝阳区光华路12号", 1.0f});
        }
        else if (poi_type == "酒店" || poi_type == "hotel")
        {
            pois.push_back({"如家酒店", "酒店", "朝阳区建国路87号", 0.5f});
            pois.push_back({"汉庭酒店", "酒店", "朝阳区东三环中路18号", 1.5f});
        }
        else if (poi_type == "医院" || poi_type == "hospital")
        {
            pois.push_back({"朝阳医院", "医院", "朝阳区工人体育场南路8号", 2.0f});
            pois.push_back({"协和医院东院", "医院", "朝阳区帅府园1号", 3.5f});
        }

        return pois;
    }

} // namespace edge_llm_rag