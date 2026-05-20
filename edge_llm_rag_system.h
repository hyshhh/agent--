#pragma once

#include <Python.h>
#include <pybind11/embed.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <string>
#include <vector>
#include <memory>
#include <filesystem>

#include <unordered_map>
#include <atomic>
#include "query_classifier.h"
#include "agent/agent_engine.h"
#include "skills/knowledge_skill.h"
#include "tools/knowledge_search_tool.h"
#include "tools/nearby_poi_tool.h"
#include "tools/weather_tool.h"
#include "tools/route_tool.h"
#include "tools/vehicle_status_tool.h"
#include "tools/emergency_tool.h"
#include "tools/maintenance_tool.h"

namespace fs = std::filesystem;

namespace py = pybind11;

namespace edge_llm_rag
{

    class EdgeLLMRAGSystem
    {
    public:
        explicit EdgeLLMRAGSystem();
        ~EdgeLLMRAGSystem();

        bool initialize();

        std::string process_query(const std::string &query,
                                  const std::string &user_id = "",
                                  const std::string &context = "");

        QueryClassification classify_query(const std::string &query);

        std::string rag_only_response(const std::string &query, bool preload = false);

        std::string llm_only_response(const std::string &query);

        std::string hybrid_response(const std::string &query);

        AgentEngine &get_agent_engine() { return *agent_engine_; }

    private:
        bool is_initialized_;

        py::object searcher;
        py::scoped_interpreter guard{};

        std::unique_ptr<QueryClassifier> query_classifier_;
        std::unique_ptr<AgentEngine> agent_engine_;

        std::shared_ptr<KnowledgeSearchTool> knowledge_search_tool_;
        std::shared_ptr<NearbyPOITool> nearby_poi_tool_;
        std::shared_ptr<WeatherTool> weather_tool_;
        std::shared_ptr<RouteTool> route_tool_;
        std::shared_ptr<VehicleStatusTool> vehicle_status_tool_;
        std::shared_ptr<EmergencyGuideTool> emergency_tool_;
        std::shared_ptr<MaintenanceTool> maintenance_tool_;

        std::unordered_map<std::string, std::string> query_cache_;

        bool add_to_cache(const std::string &query, const std::string &response);
        std::string get_from_cache(const std::string &query);
        bool is_cache_valid(const std::string &query);

        void rag_message_worker(const std::string &rag_text);
        bool preload_common_queries();

        bool initialize_agent();
        bool initialize_tools();
        bool initialize_skills();

        std::string llm_callback(const std::string &prompt);
    };

} // namespace edge_llm_rag