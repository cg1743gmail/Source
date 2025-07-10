# Unreal Engine 5.5 Interchange Plugin Python Extension

## 概述

这个扩展为Unreal Engine 5.5的自定义Interchange插件提供了完整的Python API支持，允许开发者通过Python脚本完全控制资产导入流程，实现高度自动化的工作流程。

## 特性

- 🚀 **完全Python控制**: 通过Python脚本控制整个Interchange流程
- 🔧 **无侵入式设计**: 不影响原有C++功能，完全向后兼容
- 🤖 **自动化工作流**: 支持文件监控、批量处理、实时导入
- ⚙️ **高度可配置**: JSON配置驱动，运行时动态调整
- 📊 **详细报告**: 完整的处理日志和性能统计
- 🔄 **热重载支持**: 文件修改时自动重新导入

## 目录结构

```
extend_python/
├── README.md                           # 本文档
├── cpp_modifications/                  # C++代码修改
│   ├── CustomFooTranslator_Extended.h
│   ├── CustomFooTranslator_Extended.cpp
│   ├── CustomDemoObjectNode_Extended.h
│   ├── CustomDemoObjectNode_Extended.cpp
│   ├── CustomDemoObjectPipeline_Extended.h
│   ├── CustomDemoObjectPipeline_Extended.cpp
│   ├── CustomDemoObjectFactory_Extended.h
│   ├── CustomDemoObjectFactory_Extended.cpp
│   └── PythonInterchangeManager.h
├── python_api/                         # Python API模块
│   ├── __init__.py
│   ├── core/                           # 核心API
│   │   ├── __init__.py
│   │   ├── translators.py
│   │   ├── nodes.py
│   │   ├── pipelines.py
│   │   └── factories.py
│   ├── automation/                     # 自动化工具
│   │   ├── __init__.py
│   │   ├── manager.py
│   │   ├── monitor.py
│   │   ├── batch_processor.py
│   │   └── quality_checker.py
│   ├── utils/                          # 工具函数
│   │   ├── __init__.py
│   │   ├── config.py
│   │   ├── logger.py
│   │   └── helpers.py
│   └── examples/                       # 使用示例
│       ├── __init__.py
│       ├── basic_import.py
│       ├── batch_processing.py
│       ├── automation_setup.py
│       └── custom_pipeline.py
├── documentation/                      # 详细文档
│   ├── API_Reference.md
│   ├── Quick_Start_Guide.md
│   ├── Advanced_Usage.md
│   ├── Configuration_Guide.md
│   └── Troubleshooting.md
├── tests/                              # 测试文件
│   ├── test_translators.py
│   ├── test_nodes.py
│   ├── test_pipelines.py
│   └── test_automation.py
└── config/                             # 配置文件
    ├── default_config.json
    ├── automation_rules.json
    └── quality_settings.json
```

## 快速开始

### 1. 安装和配置

```python
# 导入Python API
import sys
sys.path.append('path/to/extend_python')

from python_api import InterchangePythonAPI

# 初始化API
api = InterchangePythonAPI()
```

### 2. 基本使用

```python
# 简单文件导入
from python_api.core.translators import CustomFooTranslator

translator = CustomFooTranslator()
success = translator.import_file("C:/Assets/MyFile.foo", "/Game/ImportedAssets/")
```

### 3. 自动化工作流

```python
# 设置自动化监控
from python_api.automation.manager import AutomationManager

manager = AutomationManager()
manager.add_watch_folder("D:/GameAssets/Characters/", "characters")
manager.start_monitoring()
```

## 主要功能

### 翻译器 (Translators)
- 支持自定义文件格式
- 智能文件类型识别
- 载荷数据处理

### 节点 (Nodes)
- 自定义节点创建
- 属性管理
- 节点关系处理

### 管道 (Pipelines)
- 可配置的处理流程
- 批量处理支持
- 质量控制集成

### 工厂 (Factories)
- 资产创建和配置
- 平台特定优化
- 后处理支持

### 自动化工具
- 文件系统监控
- 批量处理器
- 质量检查器
- 报告生成器

## 版本兼容性

- Unreal Engine 5.5+
- Python 3.7+
- 原始Interchange插件完全兼容

## 许可证

与原始Interchange插件保持一致的许可证。

## 支持

如有问题或建议，请参考documentation文件夹中的详细文档。
