#pragma once

#include "base_tool.h"
#include <Python.h>
#include <pybind11/embed.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

namespace edge_llm_rag
{

    class KnowledgeSearchTool : public BaseTool
    {
    public:
        explicit KnowledgeSearchTool();
        ~KnowledgeSearchTool() override = default;

        ToolResult execute(const std::unordered_map<std::string, std::string> &params) override;
        std::string get_help() const override;

        bool initialize_searcher(const std::string &vector_db_path,
                                 const std::string &model_path);

    private:
        py::object searcher_;
        bool searcher_initialized_;
    };

} // namespace edge_llm_rag