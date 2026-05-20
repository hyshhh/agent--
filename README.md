# EdgeAgent - 边缘端智能座舱 AI 助手

<div align="center">

![C++](https://img.shields.io/badge/C++-17-blue?logo=cplusplus)
![Python](https://img.shields.io/badge/Python-3-3776AB?logo=python)
![RK3576](https://img.shields.io/badge/RK3576-NPU-green)
![License](https://img.shields.io/badge/License-MIT-yellow)

**基于 RK3576 NPU 的离线智能座舱语音交互系统**

*让每一辆车都拥有自己的 AI 大脑*

</div>

---

## 项目亮点

### 核心痛点解决

传统车载语音助手存在三大痛点：

| 痛点 | EdgeAgent 解决方案 |
|------|-------------------|
| **关键词匹配僵化** | Agent 自主决策，理解复杂语义 |
| **功能扩展困难** | 模块化 Skills + Tools，热插拔架构 |
| **单一功能调用** | 多工具动态组合，一次请求多任务处理 |

### 技术创新

```
┌─────────────────────────────────────────────────────────┐
│                    EdgeAgent 架构                        │
├─────────────────────────────────────────────────────────┤
│  用户语音 → ASR → Agent 引擎 → 决策 → Skills/Tools → TTS │
│                         │                               │
│                         ▼                               │
│              ┌─────────────────────┐                    │
│              │   双层决策架构       │                    │
│              │  ┌───────────────┐  │                    │
│              │  │ 规则快速路径   │  │  ← 70% 简单查询   │
│              │  │ (<10ms)       │  │                    │
│              │  └───────────────┘  │                    │
│              │  ┌───────────────┐  │                    │
│              │  │ LLM 复杂决策  │  │  ← 30% 复杂查询   │
│              │  │ (100-500ms)   │  │                    │
│              │  └───────────────┘  │                    │
│              └─────────────────────┘                    │
└─────────────────────────────────────────────────────────┘
```

---

## 核心能力

### 11 个 AI Skills（知识技能）

| 技能 | 功能 | 示例查询 |
|------|------|---------|
| `warning_lights` | 仪表盘警告灯解释 | "发动机灯亮了" |
| `maintenance_guide` | 车辆保养指南 | "多久保养一次" |
| `feature_guide` | 功能使用说明 | "自动泊车怎么用" |
| `troubleshooting` | 故障排除 | "车抖动怎么办" |
| `safety_driving` | 安全驾驶建议 | "雨天开车注意事项" |
| `emergency_handling` | 紧急情况处理 | "爆胎了怎么办" |
| `tech_specs` | 车辆技术参数 | "这车排量多少" |
| `fuel_economy` | 燃油经济性 | "怎么省油" |
| `maintenance_record` | 维护记录查询 | "上次保养是什么时候" |
| `warranty_policy` | 保修政策 | "保修期多长" |
| `contact_info` | 联系信息 | "4S店电话多少" |

### 7 个智能 Tools（工具）

| 工具 | 功能 | 参数 |
|------|------|------|
| `vehicle_knowledge_search` | 知识库搜索 | query, skill_name, top_k |
| `get_vehicle_status` | 车辆状态查询 | status_type |
| `emergency_guide` | 紧急处理指南 | emergency_type |
| `maintenance_reminder` | 保养提醒 | service_type |
| `nearby_poi_search` | 附近 POI 搜索 | poi_type, location |
| `route_planning` | 路线规划 | destination |
| `weather_query` | 天气查询 | location |

---

## Agent 决策流程

```
用户: "发动机故障灯亮了，附近有修车店吗"

Agent 决策链:
1. 意图分类 → KNOWLEDGE + TOOL_USE
2. 并行执行:
   - Skill: warning_lights → 获取故障说明
   - Tool: nearby_poi_search(poi_type=修车店) → 搜索附近修车店
3. LLM 整合结果
4. 返回: "发动机故障灯亮起可能表示... 附近有以下修车店..."

置信度: 0.85 | 使用工具: 1 | 使用技能: 1
```

---

## 快速开始

### 环境要求

- **硬件**: RK3576 NPU 开发板
- **系统**: Linux (ARM64)
- **依赖**: CMake 3.16+, Python 3.8+, pybind11

### 编译运行

```bash
# 克隆仓库
git clone https://github.com/hyshhh/agent--.git
cd agent--

# 编译
mkdir build && cd build
cmake ..
make -j4

# 运行
./edge_agent_demo
```

### 目录结构

```
agent--/
├── README.md                    # 项目说明
├── agent/                       # Agent 决策引擎
│   ├── agent_engine.h/.cpp      # 核心引擎
│   └── intent_classifier.h/.cpp # 意图分类器
├── skills/                      # 知识技能
│   ├── base_skill.h             # 基类
│   ├── knowledge_skill.h/.cpp   # Python 向量搜索
│   └── skill_registry.h/.cpp    # 注册中心
├── tools/                       # 工具集
│   ├── base_tool.h              # 基类
│   ├── knowledge_search_tool    # 知识搜索
│   ├── nearby_poi_tool          # 附近 POI
│   ├── weather_tool             # 天气查询
│   ├── route_tool               # 路线规划
│   ├── vehicle_status_tool      # 车辆状态
│   ├── emergency_tool           # 紧急指南
│   └── maintenance_tool         # 保养提醒
├── edge_llm_rag_system.h/.cpp   # 主系统
└── query_classifier.h/.cpp      # 查询分类器
```

---

## 技术栈

| 层级 | 技术 | 说明 |
|------|------|------|
| **AI 推理** | DeepSeek-R1-Distill-Qwen-1.5B | 1.5B 参数量，边缘端优化 |
| **向量搜索** | SentenceTransformer | 语义相似度计算 |
| **LLM SDK** | RKLLM | RK3576 NPU 加速 |
| **通信** | ZeroMQ | 进程间高效通信 |
| **TTS** | SummerTTS (VITS) | 高质量语音合成 |
| **绑定** | pybind11 | C++/Python 无缝集成 |

---

## 架构优势

### 为什么选择 EdgeAgent？

| 特性 | 传统方案 | EdgeAgent |
|------|----------|-----------|
| **响应延迟** | 500ms-2s（云端） | 10-500ms（本地） |
| **网络依赖** | 必须联网 | 完全离线 |
| **功能扩展** | 修改核心代码 | 插件化 Skills/Tools |
| **隐私安全** | 数据上传云端 | 本地处理，零泄露 |
| **成本** | 按调用计费 | 一次性部署 |

---

## 应用场景

- **智能座舱**: 语音控制车辆功能、查询车辆状态
- **车载助手**: 导航、天气、新闻、娱乐
- **紧急救援**: 故障诊断、事故处理、救援呼叫
- **保养提醒**: 智能保养周期、维修记录查询
- **驾驶辅助**: 安全驾驶建议、路况提醒

---

## 许可证

[MIT License](LICENSE)

---

<div align="center">

**如果这个项目对你有帮助，请给个 Star 支持一下！**

Made with ❤️ by [hyshhh](https://github.com/hyshhh)

</div>
