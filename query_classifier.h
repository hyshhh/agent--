#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace edge_llm_rag
{

    struct QueryClassification
    {
        enum QueryType
        {
            RAG_ONLY,
            LLM_ONLY,
            HYBRID,
            UNKNOWN_QUERY
        };

        QueryType type;
        float confidence;
        std::string reasoning;
        bool needs_context;
    };

    class QueryClassifier
    {
    public:
        QueryClassifier() = default;
        ~QueryClassifier() = default;

        QueryClassification classify_query(const std::string &query);

    private:
        std::vector<std::string> rag_keywords_ = {
            "故障", "问题", "如何", "怎么", "什么", "为什么",
            "保养", "维修", "检查", "使用", "设置"};

        std::vector<std::string> llm_keywords_ = {
            "讲个", "说说", "解释", "推荐", "建议", "比较"};
    };

} // namespace edge_llm_rag