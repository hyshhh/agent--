#include "tool_manager.h"
#include <iostream>

namespace edge_llm_rag
{

    ToolManager &ToolManager::instance()
    {
        static ToolManager manager;
        return manager;
    }

    void ToolManager::register_tool(ToolPtr tool)
    {
        if (tool)
        {
            tools_[tool->get_name()] = tool;
            std::cout << "[ToolManager] Registered tool: " << tool->get_name() << std::endl;
        }
    }

    ToolPtr ToolManager::get_tool(const std::string &name)
    {
        auto it = tools_.find(name);
        return it != tools_.end() ? it->second : nullptr;
    }

    std::vector<ToolPtr> ToolManager::get_all_tools()
    {
        std::vector<ToolPtr> result;
        for (const auto &[name, tool] : tools_)
        {
            result.push_back(tool);
        }
        return result;
    }

    std::vector<std::string> ToolManager::get_tool_names()
    {
        std::vector<std::string> names;
        for (const auto &[name, tool] : tools_)
        {
            names.push_back(name);
        }
        return names;
    }

    void ToolManager::initialize_default_tools()
    {
        std::cout << "[ToolManager] Initializing default tools..." << std::endl;
    }

} // namespace edge_llm_rag