#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include "base_tool.h"

namespace edge_llm_rag
{

    class ToolManager
    {
    public:
        static ToolManager &instance();

        void register_tool(ToolPtr tool);
        ToolPtr get_tool(const std::string &name);
        std::vector<ToolPtr> get_all_tools();
        std::vector<std::string> get_tool_names();

        void initialize_default_tools();

    private:
        ToolManager() = default;
        std::unordered_map<std::string, ToolPtr> tools_;
    };

} // namespace edge_llm_rag