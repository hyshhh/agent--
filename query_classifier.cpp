#include "query_classifier.h"
#include <algorithm>

namespace edge_llm_rag
{

    QueryClassification QueryClassifier::classify_query(const std::string &query)
    {
        QueryClassification result;
        result.confidence = 0.0f;
        result.needs_context = false;
        result.type = QueryClassification::UNKNOWN_QUERY;

        int rag_score = 0;
        int llm_score = 0;

        for (const auto &keyword : rag_keywords_)
        {
            if (query.find(keyword) != std::string::npos)
            {
                rag_score++;
            }
        }

        for (const auto &keyword : llm_keywords_)
        {
            if (query.find(keyword) != std::string::npos)
            {
                llm_score++;
            }
        }

        if (rag_score > llm_score && rag_score > 0)
        {
            result.type = QueryClassification::RAG_ONLY;
            result.confidence = std::min(0.5f + rag_score * 0.1f, 0.9f);
            result.reasoning = "匹配到RAG关键词";
            result.needs_context = true;
        }
        else if (llm_score > rag_score && llm_score > 0)
        {
            result.type = QueryClassification::LLM_ONLY;
            result.confidence = std::min(0.5f + llm_score * 0.1f, 0.9f);
            result.reasoning = "匹配到LLM关键词";
        }
        else if (rag_score > 0 && llm_score > 0)
        {
            result.type = QueryClassification::HYBRID;
            result.confidence = 0.7f;
            result.reasoning = "同时匹配到RAG和LLM关键词";
            result.needs_context = true;
        }
        else
        {
            result.type = QueryClassification::HYBRID;
            result.confidence = 0.5f;
            result.reasoning = "未匹配到明确关键词，使用混合模式";
        }

        return result;
    }

} // namespace edge_llm_rag