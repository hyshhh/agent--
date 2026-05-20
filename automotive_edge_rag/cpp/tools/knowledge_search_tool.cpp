#include "knowledge_search_tool.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace edge_llm_rag
{

    KnowledgeSearchTool::KnowledgeSearchTool()
        : BaseTool("vehicle_knowledge_search", "搜索车辆知识库获取相关信息"),
          searcher_initialized_(false)
    {
        add_parameter({"query", "string", "搜索查询内容", true, ""});
        add_parameter({"skill_name", "string", "指定搜索的技能领域（可选）", false, ""});
        add_parameter({"top_k", "string", "返回结果数量（可选，默认1）", false, "1"});
    }

    bool KnowledgeSearchTool::initialize_searcher(const std::string &vector_db_path,
                                                   const std::string &model_path)
    {
        try
        {
            py::module_ sys = py::module_::import("sys");
            py::list path = sys.attr("path");
            path.append("python");

            py::module_ mod = py::module_::import("vehicle_vector_search");
            searcher_ = mod.attr("VehicleVectorSearch")(vector_db_path);

            auto load_t0 = std::chrono::high_resolution_clock::now();
            searcher_.attr("load_model")(model_path);
            auto load_t1 = std::chrono::high_resolution_clock::now();
            double load_ms = std::chrono::duration<double, std::milli>(load_t1 - load_t0).count();

            std::cout << "[KnowledgeSearchTool] Model loaded ("
                      << std::fixed << std::setprecision(2) << load_ms << " ms)" << std::endl;

            searcher_initialized_ = true;
            return true;
        }
        catch (const std::exception &e)
        {
            std::cerr << "[KnowledgeSearchTool] Failed to initialize: " << e.what() << std::endl;
            return false;
        }
    }

    ToolResult KnowledgeSearchTool::execute(const std::unordered_map<std::string, std::string> &params)
    {
        ToolResult result;
        result.tool_name = name_;
        result.success = false;

        if (!searcher_initialized_)
        {
            result.error_message = "搜索引擎未初始化";
            return result;
        }

        auto it = params.find("query");
        if (it == params.end())
        {
            result.error_message = "缺少必要参数: query";
            return result;
        }

        std::string query = it->second;
        int top_k = 1;

        it = params.find("top_k");
        if (it != params.end())
        {
            try
            {
                top_k = std::stoi(it->second);
            }
            catch (...)
            {
                top_k = 1;
            }
        }

        try
        {
            float threshold = 0.5f;

            auto t0 = std::chrono::high_resolution_clock::now();
            py::object results = searcher_.attr("search")(query, top_k, threshold);
            auto t1 = std::chrono::high_resolution_clock::now();
            double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

            std::cout << "[KnowledgeSearchTool] Search completed in "
                      << std::fixed << std::setprecision(2) << ms << " ms" << std::endl;

            if (py::len(results) == 0)
            {
                result.content = "未找到相关信息";
                return result;
            }

            std::ostringstream oss;
            oss << "搜索结果：\n\n";

            for (const auto &item : results)
            {
                double sim = item["similarity"].cast<double>();
                std::string text = item["text"].cast<std::string>();
                std::string section = item["section"].cast<std::string>();
                std::string subsection = item["subsection"].cast<std::string>();

                oss << "相关度: " << std::fixed << std::setprecision(4) << sim << "\n";
                oss << "来源: " << section << (subsection.empty() ? "" : "/" + subsection) << "\n";
                oss << "内容: " << text << "\n\n";
            }

            result.content = oss.str();
            result.success = true;
        }
        catch (const std::exception &e)
        {
            result.error_message = "搜索出错: " + std::string(e.what());
            std::cerr << "[KnowledgeSearchTool] Error: " << e.what() << std::endl;
        }

        return result;
    }

    std::string KnowledgeSearchTool::get_help() const
    {
        return description_ + "\n"
               "参数:\n"
               "  query (必需): 搜索查询内容\n"
               "  skill_name (可选): 指定搜索的技能领域\n"
               "  top_k (可选): 返回结果数量\n"
               "示例: vehicle_knowledge_search(query=发动机故障灯)";
    }

} // namespace edge_llm_rag