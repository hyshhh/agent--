#pragma once
#include "edge_llm_rag_system.h"
#include "query_classifier.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <random>
#include <iomanip>
#include <regex>
#include <cwchar>
#include <cstdlib>
#include <codecvt>

namespace edge_llm_rag
{
    EdgeLLMRAGSystem::EdgeLLMRAGSystem()
        : is_initialized_(false)
    {

        py::module_ sys = py::module_::import("sys");
        py::list path = sys.attr("path");
        path.append("python");

        py::module_ mod = py::module_::import("vehicle_vector_search");
        searcher = mod.attr("VehicleVectorSearch")("vector_db");

        std::cout << "Loading model once..." << std::endl;
        auto load_t0 = std::chrono::high_resolution_clock::now();

        fs::path cpp_dir = fs::absolute(__FILE__).parent_path();
        fs::path model_path = cpp_dir.parent_path() / "models";
        searcher.attr("load_model")(model_path.string());
        auto load_t1 = std::chrono::high_resolution_clock::now();
        double load_ms = std::chrono::duration<double, std::milli>(load_t1 - load_t0).count();
        std::cout << "Model loaded (" << std::fixed << std::setprecision(2) << load_ms << " ms)" << std::endl;

        py::object stats = searcher.attr("get_statistics")();
        std::cout << "Stats: total_documents=" << stats["total_documents"].cast<int>()
                  << ", embedding_dimension=" << stats["embedding_dimension"].cast<int>()
                  << std::endl;
    }

    EdgeLLMRAGSystem::~EdgeLLMRAGSystem()
    {
        
    }

    bool EdgeLLMRAGSystem::initialize()
    {
        try
        {
            query_classifier_ = std::make_unique<QueryClassifier>();

            if (!initialize_agent())
            {
                std::cerr << "Agent初始化失败" << std::endl;
                return false;
            }

            query_cache_.clear();

            is_initialized_ = true;
            std::cout << "系统初始化成功" << std::endl;
            return true;
        }
        catch (const std::exception &e)
        {
            std::cerr << "系统初始化失败: " << e.what() << std::endl;
            return false;
        }
    }

    std::string EdgeLLMRAGSystem::process_query(const std::string &query,
                                                const std::string &user_id,
                                                const std::string &context)
    {
        if (!is_initialized_)
        {
            return "系统未初始化";
        }

        std::string cached_response = get_from_cache(query);
        if (!cached_response.empty())
        {
            return cached_response;
        }

        std::cout << "\n================================" << std::endl;
        std::cout << "Agent处理查询: " << query << std::endl;
        std::cout << "================================" << std::endl;

        AgentResponse agent_response = agent_engine_->process(query);

        std::cout << "[Agent] 使用的工具: ";
        for (const auto &tool : agent_response.tools_used)
        {
            std::cout << tool << " ";
        }
        std::cout << std::endl;

        std::cout << "[Agent] 使用的技能: ";
        for (const auto &skill : agent_response.skills_used)
        {
            std::cout << skill << " ";
        }
        std::cout << std::endl;

        std::cout << "[Agent] LLM决策: " << (agent_response.used_llm ? "是" : "否") << std::endl;
        std::cout << "[Agent] 置信度: " << agent_response.confidence << std::endl;

        std::string response = agent_response.content;

        rag_message_worker(response);

        add_to_cache(query, response);

        return response;
    }

    QueryClassification EdgeLLMRAGSystem::classify_query(const std::string &query)
    {
        if (!query_classifier_)
        {
            return QueryClassification{QueryClassification::UNKNOWN_QUERY, 0.0f, "分类器未初始化", false};
        }

        return query_classifier_->classify_query(query);
    }


    void EdgeLLMRAGSystem::rag_message_worker(const std::string &rag_text)
    {
        static const std::wregex wide_delimiter(
            L"([。！？；：\n]|\\?\\s|\\!\\s|\\；|\\，|\\、|\\|)");
        const std::wstring END_MARKER = L"END";

        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

        std::wstring wide_text = converter.from_bytes(rag_text) + END_MARKER;

        std::wsregex_iterator it(wide_text.begin(), wide_text.end(), wide_delimiter);
        std::wsregex_iterator end;

        int skip_counter = 0;
        size_t last_pos = 0;
        while (it != end && skip_counter < 2)
        {
            last_pos = it->position() + it->length();
            ++it;
            ++skip_counter;
        }

        while (it != end)
        {
            size_t seg_start = last_pos;
            size_t seg_end = it->position();
            last_pos = seg_end + it->length();

            std::wstring wide_segment = wide_text.substr(seg_start, seg_end - seg_start);

            wide_segment.erase(0, wide_segment.find_first_not_of(L" \t\n\r"));
            wide_segment.erase(wide_segment.find_last_not_of(L" \t\n\r") + 1);

            if (!wide_segment.empty())
            {
                auto response1 = tts_client_.request(converter.to_bytes(wide_segment));
                std::cout << "[tts -> RAG] received: " << response1 << std::endl;
            }
            ++it;
        }

        if (last_pos < wide_text.length())
        {
            std::wstring last_segment = wide_text.substr(last_pos);
            if (!last_segment.empty())
            {
                auto response1 = tts_client_.request(converter.to_bytes(last_segment));
                std::cout << "[tts -> RAG] received: " << response1 << std::endl;
            }
        }
    }

    std::string EdgeLLMRAGSystem::rag_only_response(const std::string &query, bool preload)
    {
        auto t0 = std::chrono::high_resolution_clock::now();
        py::object results = searcher.attr("search")(query, 1, 0.5);
        auto t1 = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

        std::cout << "\nQuery: '" << query << "'\n";
        std::cout
            << "elapsed: " << std::fixed << std::setprecision(2) << ms << " ms\n";

        if (py::len(results) == 0)
        {
            std::cout << "  No results" << std::endl;
            return "No results !!!";
        }

        std::string answer;
        for (const auto &item : results)
        {
            double sim = item["similarity"].cast<double>();
            std::string text = item["text"].cast<std::string>();
            answer = item["text"].cast<std::string>();
            std::string section = item["section"].cast<std::string>();
            std::string subsection = item["subsection"].cast<std::string>();
            std::cout << "  sim=" << std::fixed << std::setprecision(4) << sim
                      << ", section=" << section
                      << (subsection.empty() ? "" : ("/" + subsection))
                      << ", text=" << text.substr(0, 100) << "...\n";
        }

        if (!preload)
        {
            rag_message_worker(answer);
        }

        return answer;
    }

    std::string EdgeLLMRAGSystem::llm_only_response(const std::string &query)
    {
        auto response = llm_client_.request(query);
        std::cout << "[tts -> RAG] received: " << response << std::endl;
        return response;
    }

    std::string EdgeLLMRAGSystem::hybrid_response(const std::string &query)
    {
        std::string rag_part = rag_only_response(query,true);
        if (rag_part.find("No results") != std::string::npos){
            return llm_only_response(query);
        }
        std::string llm_query = query + "<rag>" + rag_part;
        std::string llm_part = llm_only_response(llm_query);

        return llm_part;
    }

    bool EdgeLLMRAGSystem::add_to_cache(const std::string &query, const std::string &response)
    {
        if (query_cache_.size() >= 100)
        {
            query_cache_.clear();
        }

        query_cache_[query] = response;
        return true;
    }

    std::string EdgeLLMRAGSystem::get_from_cache(const std::string &query)
    {
        auto it = query_cache_.find(query);
        if (it != query_cache_.end())
        {
            return it->second;
        }
        return "";
    }

    bool EdgeLLMRAGSystem::is_cache_valid(const std::string &query)
    {
        return query_cache_.find(query) != query_cache_.end();
    }

    bool EdgeLLMRAGSystem::preload_common_queries()
    {
        std::vector<std::string> common_queries = {
            "发动机故障",
            "制动系统",
            "空调不制冷",
            "保养周期"};

        for (const auto &query : common_queries)
        {
            if (query_cache_.find(query) == query_cache_.end())
            {
                add_to_cache(query, rag_only_response(query, true));
            }
        }

        return true;
    }

    bool EdgeLLMRAGSystem::initialize_agent()
    {
        std::cout << "初始化Agent引擎..." << std::endl;

        agent_engine_ = std::make_unique<AgentEngine>();

        agent_engine_->set_llm_callback([this](const std::string &prompt) -> std::string
                                        { return llm_callback(prompt); });

        if (!initialize_tools())
        {
            std::cerr << "工具初始化失败" << std::endl;
            return false;
        }

        if (!initialize_skills())
        {
            std::cerr << "技能初始化失败" << std::endl;
            return false;
        }

        std::cout << "Agent引擎初始化成功" << std::endl;
        return true;
    }

    bool EdgeLLMRAGSystem::initialize_tools()
    {
        std::cout << "初始化工具..." << std::endl;

        fs::path cpp_dir = fs::absolute(__FILE__).parent_path();
        fs::path model_path = cpp_dir.parent_path() / "models";
        std::string vector_db_path = "vector_db";
        std::string model_path_str = model_path.string();

        knowledge_search_tool_ = std::make_shared<KnowledgeSearchTool>();
        if (!knowledge_search_tool_->initialize_searcher(vector_db_path, model_path_str))
        {
            std::cerr << "知识搜索工具初始化失败" << std::endl;
            return false;
        }
        agent_engine_->register_tool(knowledge_search_tool_);

        nearby_poi_tool_ = std::make_shared<NearbyPOITool>();
        agent_engine_->register_tool(nearby_poi_tool_);

        weather_tool_ = std::make_shared<WeatherTool>();
        agent_engine_->register_tool(weather_tool_);

        route_tool_ = std::make_shared<RouteTool>();
        agent_engine_->register_tool(route_tool_);

        vehicle_status_tool_ = std::make_shared<VehicleStatusTool>();
        agent_engine_->register_tool(vehicle_status_tool_);

        emergency_tool_ = std::make_shared<EmergencyGuideTool>();
        agent_engine_->register_tool(emergency_tool_);

        maintenance_tool_ = std::make_shared<MaintenanceTool>();
        agent_engine_->register_tool(maintenance_tool_);

        std::cout << "工具初始化完成，共注册 " << agent_engine_->get_available_tools_schema().size() << " 个工具" << std::endl;
        return true;
    }

    bool EdgeLLMRAGSystem::initialize_skills()
    {
        std::cout << "初始化技能..." << std::endl;

        fs::path cpp_dir = fs::absolute(__FILE__).parent_path();
        fs::path model_path = cpp_dir.parent_path() / "models";
        std::string vector_db_path = "vector_db";
        std::string model_path_str = model_path.string();

        auto warning_lights_skill = std::make_shared<KnowledgeSkill>(
            "warning_lights",
            "仪表盘警告灯解释和处理建议",
            "warning_lights",
            std::vector<std::string>{"警告灯", "指示灯", "故障灯", "仪表盘", "亮灯",
                                     "发动机灯", "ABS灯", "气囊灯", "机油灯", "水温灯"});
        if (warning_lights_skill->initialize_searcher(vector_db_path, model_path_str))
        {
            agent_engine_->register_skill(warning_lights_skill);
        }

        auto maintenance_guide_skill = std::make_shared<KnowledgeSkill>(
            "maintenance_guide",
            "车辆保养指南和建议",
            "maintenance",
            std::vector<std::string>{"保养", "维护", "更换", "检查", "清洁",
                                     "机油", "滤清器", "火花塞", "制动片"});
        if (maintenance_guide_skill->initialize_searcher(vector_db_path, model_path_str))
        {
            agent_engine_->register_skill(maintenance_guide_skill);
        }

        auto feature_guide_skill = std::make_shared<KnowledgeSkill>(
            "feature_guide",
            "车辆功能使用说明",
            "feature",
            std::vector<std::string>{"功能", "使用", "操作", "设置", "调节",
                                     "自动泊车", "车道保持", "定速巡航", "导航"});
        if (feature_guide_skill->initialize_searcher(vector_db_path, model_path_str))
        {
            agent_engine_->register_skill(feature_guide_skill);
        }

        auto troubleshooting_skill = std::make_shared<KnowledgeSkill>(
            "troubleshooting",
            "故障排除指南",
            "troubleshooting",
            std::vector<std::string>{"排除", "诊断", "检查", "问题", "原因",
                                     "无法启动", "异响", "抖动", "漏水"});
        if (troubleshooting_skill->initialize_searcher(vector_db_path, model_path_str))
        {
            agent_engine_->register_skill(troubleshooting_skill);
        }

        auto safety_driving_skill = std::make_shared<KnowledgeSkill>(
            "safety_driving",
            "安全驾驶建议",
            "safety",
            std::vector<std::string>{"安全", "驾驶", "注意", "建议", "提醒",
                                     "高速", "雨天", "夜间", "冬季"});
        if (safety_driving_skill->initialize_searcher(vector_db_path, model_path_str))
        {
            agent_engine_->register_skill(safety_driving_skill);
        }

        auto emergency_handling_skill = std::make_shared<KnowledgeSkill>(
            "emergency_handling",
            "紧急情况处理指南",
            "emergency",
            std::vector<std::string>{"紧急", "处理", "应对", "措施", "救援",
                                     "爆胎", "没油", "抛锚", "事故"});
        if (emergency_handling_skill->initialize_searcher(vector_db_path, model_path_str))
        {
            agent_engine_->register_skill(emergency_handling_skill);
        }

        auto tech_specs_skill = std::make_shared<KnowledgeSkill>(
            "tech_specs",
            "车辆技术参数",
            "specs",
            std::vector<std::string>{"参数", "规格", "数据", "尺寸", "重量",
                                     "排量", "功率", "扭矩", "轴距"});
        if (tech_specs_skill->initialize_searcher(vector_db_path, model_path_str))
        {
            agent_engine_->register_skill(tech_specs_skill);
        }

        auto fuel_economy_skill = std::make_shared<KnowledgeSkill>(
            "fuel_economy",
            "燃油经济性信息",
            "fuel",
            std::vector<std::string>{"油耗", "燃油", "经济", "省油", "加油",
                                     "续航", "里程", "油箱"});
        if (fuel_economy_skill->initialize_searcher(vector_db_path, model_path_str))
        {
            agent_engine_->register_skill(fuel_economy_skill);
        }

        auto maintenance_record_skill = std::make_shared<KnowledgeSkill>(
            "maintenance_record",
            "车辆维护记录",
            "record",
            std::vector<std::string>{"记录", "历史", "保养记录", "维修记录",
                                     "上次", "下次", "周期"});
        if (maintenance_record_skill->initialize_searcher(vector_db_path, model_path_str))
        {
            agent_engine_->register_skill(maintenance_record_skill);
        }

        auto warranty_policy_skill = std::make_shared<KnowledgeSkill>(
            "warranty_policy",
            "保修政策信息",
            "warranty",
            std::vector<std::string>{"保修", "质保", "条款", "政策",
                                     "保修期", "保修范围", "免费维修"});
        if (warranty_policy_skill->initialize_searcher(vector_db_path, model_path_str))
        {
            agent_engine_->register_skill(warranty_policy_skill);
        }

        auto contact_info_skill = std::make_shared<KnowledgeSkill>(
            "contact_info",
            "联系信息",
            "contact",
            std::vector<std::string>{"联系", "电话", "客服", "救援", "咨询",
                                     "4S店", "经销商", "服务热线"});
        if (contact_info_skill->initialize_searcher(vector_db_path, model_path_str))
        {
            agent_engine_->register_skill(contact_info_skill);
        }

        std::cout << "技能初始化完成" << std::endl;
        return true;
    }

    std::string EdgeLLMRAGSystem::llm_callback(const std::string &prompt)
    {
        auto response = llm_client_.request(prompt);
        std::cout << "[LLM Callback] Response received" << std::endl;
        return response;
    }

} // namespace edge_llm_rag