# Edge_LLM_RAG_Voice - Agent + Skills 架构

基于 RK3576 边缘端的离线智能座舱语音交互系统，采用 Agent + Skills 自主决策架构。

## 项目简介

这是一个车载离线语音交互系统，集成了 ASR、RAG、LLM 和 TTS 四大模块。系统采用 Agent + Skills 架构，让 LLM 自主决策调用哪些工具，支持动态工具组合和多技能协作。

## 核心特性

- **Agent 自主决策**：LLM 根据用户问题自主决定是否调用工具
- **双层决策架构**：规则快速路径（70%）+ LLM 复杂决策（30%）
- **11 个 Skills**：覆盖车辆知识的各个领域
- **7 个 Tools**：支持知识搜索、车辆状态、紧急指南等功能
- **边缘端部署**：基于 RK3576 NPU，无需云端依赖

## 目录结构

```
├── agent/                    # Agent 决策引擎
│   ├── agent_engine.h/cpp    # 核心引擎
│   └── intent_classifier.h/cpp # 意图分类器
├── skills/                   # 知识技能
│   ├── base_skill.h          # 基类
│   ├── knowledge_skill.h/cpp # Python 向量搜索
│   └── skill_registry.h/cpp  # 注册中心
├── tools/                    # 工具集
│   ├── base_tool.h           # 基类
│   ├── knowledge_search_tool # 知识搜索
│   ├── nearby_poi_tool       # 附近 POI
│   ├── weather_tool          # 天气查询
│   ├── route_tool            # 路线规划
│   ├── vehicle_status_tool   # 车辆状态
│   ├── emergency_tool        # 紧急指南
│   └── maintenance_tool      # 保养提醒
├── edge_llm_rag_system.h/cpp # 主系统
└── query_classifier.h/cpp    # 查询分类器
```

## 技术栈

- **语言**：C++17、Python 3
- **AI 框架**：RKLLM SDK、SentenceTransformer
- **通信**：ZeroMQ
- **构建**：CMake
- **硬件**：RK3576 NPU

## 快速开始

```bash
# 编译
mkdir build && cd build
cmake ..
make -j4

# 运行
./edge_llm_rag_demo
```

## 许可证

MIT License