#include "knowledge_skill.h"
#include <iostream>
#include <chrono>
#include <iomanip>

namespace edge_llm_rag
{

    KnowledgeSkill::KnowledgeSkill(const std::string &name,
                                   const std::string &description,
                                   const std::string &domain,
                                   const std::vector<std::string> &keywords)
        : BaseSkill(name, description, domain),
          searcher_initialized_(false)
    {
        keywords_ = keywords;
    }

    bool KnowledgeSkill::initialize_searcher(const std::string &vector_db_path,
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

            std::cout << "[KnowledgeSkill:" << name_ << "] Model loaded ("
                      << std::fixed << std::setprecision(2) << load_ms << " ms)" << std::endl;

            searcher_initialized_ = true;
            return true;
        }
        catch (const std::exception &e)
        {
            std::cerr << "[KnowledgeSkill:" << name_ << "] Failed to initialize: "
                      << e.what() << std::endl;
            return false;
        }
    }

    SkillResult KnowledgeSkill::execute(const std::string &query,
                                         const std::unordered_map<std::string, std::string> &params)
    {
        SkillResult result;
        result.skill_name = name_;
        result.success = false;

        if (!searcher_initialized_)
        {
            result.content = "搜索引擎未初始化";
            return result;
        }

        try
        {
            int top_k = 1;
            float threshold = 0.5f;

            auto it = params.find("top_k");
            if (it != params.end())
            {
                top_k = std::stoi(it->second);
            }

            it = params.find("threshold");
            if (it != params.end())
            {
                threshold = std::stof(it->second);
            }

            auto t0 = std::chrono::high_resolution_clock::now();
            py::object results = searcher_.attr("search")(query, top_k, threshold);
            auto t1 = std::chrono::high_resolution_clock::now();
            double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

            std::cout << "[KnowledgeSkill:" << name_ << "] Search completed in "
                      << std::fixed << std::setprecision(2) << ms << " ms" << std::endl;

            if (py::len(results) == 0)
            {
                result.content = "未找到相关信息";
                result.relevance_score = 0.0f;
                return result;
            }

            std::string combined_content;
            float max_similarity = 0.0f;

            for (const auto &item : results)
            {
                double sim = item["similarity"].cast<double>();
                std::string text = item["text"].cast<std::string>();
                std::string section = item["section"].cast<std::string>();
                std::string subsection = item["subsection"].cast<std::string>();

                if (sim > max_similarity)
                {
                    max_similarity = static_cast<float>(sim);
                }

                combined_content += text + "\n";

                result.sources.push_back(section + (subsection.empty() ? "" : "/" + subsection));

                std::cout << "  sim=" << std::fixed << std::setprecision(4) << sim
                          << ", section=" << section
                          << (subsection.empty() ? "" : "/" + subsection) << std::endl;
            }

            result.content = combined_content;
            result.relevance_score = max_similarity;
            result.success = true;
        }
        catch (const std::exception &e)
        {
            result.content = "搜索出错: " + std::string(e.what());
            std::cerr << "[KnowledgeSkill:" << name_ << "] Error: " << e.what() << std::endl;
        }

        return result;
    }

    std::string KnowledgeSkill::get_help() const
    {
        return "知识库搜索技能 - " + description_ + "\n"
               "领域: " + domain_ + "\n"
               "触发关键词: " + std::to_string(keywords_.size()) + "个";
    }

} // namespace edge_llm_rag