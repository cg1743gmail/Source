# Unreal Engine 5.5 Interchange插件Python API集成指南

## 1. 可行性分析

### 1.1 现有Python API支持
Unreal Engine 5.5已经为Interchange系统提供了基础的Python API支持，主要包括：

```python
import unreal

# 核心Interchange类
interchange_manager = unreal.InterchangeManager.get_interchange_manager_scripted()
source_data = unreal.InterchangeManager.create_source_data(file_path)
```

这些API允许基本的资产导入操作，但目前**不包含对自定义翻译器、节点和管道的完整访问**。

### 1.2 自定义插件API暴露可行性

基于对现有代码的分析，将自定义Interchange插件API暴露给Python是完全可行的，主要通过以下机制：

1. **UCLASS宏标记**: 所有关键类已使用`UCLASS(BlueprintType)`标记，这是Python绑定的基础
2. **UFUNCTION宏**: 可以为关键方法添加`UFUNCTION(BlueprintCallable)`使其在Python中可用
3. **UPROPERTY宏**: 可以为属性添加`UPROPERTY(BlueprintReadWrite)`使其在Python中可访问

## 2. 核心API暴露方案

### 2.1 翻译器API暴露

```cpp
// 修改前
UCLASS(BlueprintType)
class CUSTOMIMPORT_API UCustomFooTranslator : public UInterchangeTranslatorBase, 
                                             public ICustomDemoObjectPayloadInterface
{
    GENERATED_BODY()
public:
    virtual TArray<FString> GetSupportedFormats() const override;
    virtual bool Translate(UInterchangeBaseNodeContainer& BaseNodeContainer) const override;
    virtual TOptional<UE::Interchange::FCustomDemoObjectData> GetDemoObjectPayloadData(const FString& PayloadKey) const override;
};

// 修改后 - 添加Python支持
UCLASS(BlueprintType)
class CUSTOMIMPORT_API UCustomFooTranslator : public UInterchangeTranslatorBase, 
                                             public ICustomDemoObjectPayloadInterface
{
    GENERATED_BODY()
public:
    // 暴露给Python的格式获取方法
    UFUNCTION(BlueprintCallable, Category = "Interchange|Translator|Custom")
    virtual TArray<FString> GetSupportedFormats() const override;
    
    // 翻译方法 - 需要包装以便Python调用
    virtual bool Translate(UInterchangeBaseNodeContainer& BaseNodeContainer) const override;
    
    // Python友好的翻译方法包装
    UFUNCTION(BlueprintCallable, Category = "Interchange|Translator|Custom")
    bool TranslateFromPath(const FString& FilePath, const FString& DestinationPath);
    
    // 载荷数据接口 - 需要包装
    virtual TOptional<UE::Interchange::FCustomDemoObjectData> GetDemoObjectPayloadData(const FString& PayloadKey) const override;
    
    // Python友好的载荷数据获取
    UFUNCTION(BlueprintCallable, Category = "Interchange|Translator|Custom")
    FString GetPayloadDataAsJson(const FString& PayloadKey) const;
};
```

### 2.2 节点API暴露

```cpp
// 修改前
UCLASS(BlueprintType)
class CUSTOMNODES_API UCustomDemoObjectNode : public UInterchangeBaseNode
{
    GENERATED_BODY()
    
public:
    void InitializeCustomDemoObjectNode(const FString& UniqueID, const FString& DisplayLabel);
    virtual FString GetTypeName() const override { return TEXT("CustomDemoObjectNode"); }
    virtual const TOptional<FString> GetPayLoadKey() const;
    virtual void SetPayLoadKey(const FString& PayLoadKey);
};

// 修改后 - 添加Python支持
UCLASS(BlueprintType)
class CUSTOMNODES_API UCustomDemoObjectNode : public UInterchangeBaseNode
{
    GENERATED_BODY()
    
public:
    // 初始化方法 - 已经是UFUNCTION
    UFUNCTION(BlueprintCallable, Category = "Interchange|Node|Custom")
    void InitializeCustomDemoObjectNode(const FString& UniqueID, const FString& DisplayLabel);
    
    // 类型名称 - 添加Python访问
    UFUNCTION(BlueprintCallable, Category = "Interchange|Node|Custom")
    virtual FString GetTypeName() const override { return TEXT("CustomDemoObjectNode"); }
    
    // 载荷键获取 - 需要包装
    virtual const TOptional<FString> GetPayLoadKey() const;
    
    // Python友好的载荷键获取
    UFUNCTION(BlueprintCallable, Category = "Interchange|Node|Custom")
    FString GetPayLoadKeyForPython() const;
    
    // 载荷键设置 - 已经是UFUNCTION
    UFUNCTION(BlueprintCallable, Category = "Interchange|Node|Custom")
    virtual void SetPayLoadKey(const FString& PayLoadKey);
    
    // 添加Python友好的属性访问方法
    UFUNCTION(BlueprintCallable, Category = "Interchange|Node|Custom")
    bool SetCustomAttribute(const FString& AttributeName, const FString& AttributeValue);
    
    UFUNCTION(BlueprintCallable, Category = "Interchange|Node|Custom")
    bool GetCustomAttribute(const FString& AttributeName, FString& OutAttributeValue) const;
};
```

### 2.3 管道API暴露

```cpp
// 修改前
UCLASS(BlueprintType, editinlinenew)
class CUSTOMPIPELINES_API UCustomDemoObjectPipeline : public UInterchangePipelineBase
{
    GENERATED_BODY()
    
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DemoObjects")
    bool bImportDemoObjects = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DemoObjects")
    bool bAssetSettingBoolean = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DemoObjects")
    float AssetSettingFloat = 0.0f;
    
    virtual void ExecutePipeline(UInterchangeBaseNodeContainer* InBaseNodeContainer, 
                                const TArray<UInterchangeSourceData*>& InSourceDatas, 
                                const FString& ContentBasePath) override;
};

// 修改后 - 添加Python支持
UCLASS(BlueprintType, editinlinenew)
class CUSTOMPIPELINES_API UCustomDemoObjectPipeline : public UInterchangePipelineBase
{
    GENERATED_BODY()
    
public:
    // 配置属性 - 已经可以从Python访问
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DemoObjects")
    bool bImportDemoObjects = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DemoObjects")
    bool bAssetSettingBoolean = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DemoObjects")
    float AssetSettingFloat = 0.0f;
    
    // 执行管道 - 需要包装
    virtual void ExecutePipeline(UInterchangeBaseNodeContainer* InBaseNodeContainer, 
                                const TArray<UInterchangeSourceData*>& InSourceDatas, 
                                const FString& ContentBasePath) override;
    
    // Python友好的管道执行
    UFUNCTION(BlueprintCallable, Category = "Interchange|Pipeline|Custom")
    bool ExecutePipelineFromPython(const TArray<FString>& FilePaths, const FString& DestinationPath);
    
    // 添加Python配置方法
    UFUNCTION(BlueprintCallable, Category = "Interchange|Pipeline|Custom")
    void ConfigurePipelineFromPython(const TMap<FString, FString>& ConfigOptions);
};
```

### 2.4 工厂API暴露

```cpp
// 修改前
UCLASS(BlueprintType)
class CUSTOMIMPORT_API UCustomDemoObjectFactory : public UInterchangeFactoryBase
{
    GENERATED_BODY()
    
public:
    virtual UClass* GetFactoryClass() const override;
    virtual EInterchangeFactoryAssetType GetFactoryAssetType() override;
    virtual FImportAssetResult BeginImportAsset_GameThread(const FImportAssetObjectParams& Arguments) override;
    virtual FImportAssetResult ImportAsset_Async(const FImportAssetObjectParams& Arguments) override;
    virtual void SetupObject_GameThread(const FSetupObjectParams& Arguments) override;
};

// 修改后 - 添加Python支持
UCLASS(BlueprintType)
class CUSTOMIMPORT_API UCustomDemoObjectFactory : public UInterchangeFactoryBase
{
    GENERATED_BODY()
    
public:
    // 工厂类获取 - 添加Python访问
    UFUNCTION(BlueprintCallable, Category = "Interchange|Factory|Custom")
    virtual UClass* GetFactoryClass() const override;
    
    // 资产类型获取 - 添加Python访问
    UFUNCTION(BlueprintCallable, Category = "Interchange|Factory|Custom")
    virtual EInterchangeFactoryAssetType GetFactoryAssetType() override;
    
    // 导入方法 - 内部实现
    virtual FImportAssetResult BeginImportAsset_GameThread(const FImportAssetObjectParams& Arguments) override;
    virtual FImportAssetResult ImportAsset_Async(const FImportAssetObjectParams& Arguments) override;
    virtual void SetupObject_GameThread(const FSetupObjectParams& Arguments) override;
    
    // Python友好的工厂方法
    UFUNCTION(BlueprintCallable, Category = "Interchange|Factory|Custom")
    UObject* CreateAssetFromPython(const FString& AssetPath, const FString& AssetName);
    
    // 自定义设置方法
    UFUNCTION(BlueprintCallable, Category = "Interchange|Factory|Custom")
    void ConfigureFactoryFromPython(const TMap<FString, FString>& FactorySettings);
};
```

## 3. Python包装层实现

### 3.1 Python模块结构

```python
# custom_interchange/__init__.py
from .translators import CustomFooTranslator
from .nodes import CustomDemoObjectNode
from .pipelines import CustomDemoObjectPipeline
from .factories import CustomDemoObjectFactory

# 版本信息
__version__ = "1.0.0"
```

### 3.2 翻译器包装示例

```python
# custom_interchange/translators.py
import unreal

class CustomFooTranslator:
    """Python包装类，用于访问UCustomFooTranslator"""
    
    @staticmethod
    def create():
        """创建翻译器实例"""
        return unreal.CustomFooTranslator()
    
    @staticmethod
    def get_supported_formats():
        """获取支持的格式列表"""
        translator = unreal.CustomFooTranslator()
        return translator.get_supported_formats()
    
    @staticmethod
    def translate_file(file_path, destination_path):
        """翻译文件并导入到指定路径"""
        translator = unreal.CustomFooTranslator()
        return translator.translate_from_path(file_path, destination_path)
    
    @staticmethod
    def get_payload_data(payload_key):
        """获取载荷数据"""
        translator = unreal.CustomFooTranslator()
        json_data = translator.get_payload_data_as_json(payload_key)
        # 可以进一步解析JSON
        import json
        return json.loads(json_data)
```

### 3.3 节点包装示例

```python
# custom_interchange/nodes.py
import unreal

class CustomDemoObjectNode:
    """Python包装类，用于访问UCustomDemoObjectNode"""
    
    def __init__(self, node_instance=None):
        """初始化节点包装器"""
        self._node = node_instance if node_instance else None
    
    @staticmethod
    def create(unique_id, display_label):
        """创建新节点"""
        # 需要通过容器创建节点
        container = unreal.InterchangeBaseNodeContainer()
        node = unreal.CustomDemoObjectNode()
        node.initialize_custom_demo_object_node(unique_id, display_label)
        wrapper = CustomDemoObjectNode(node)
        return wrapper
    
    def get_type_name(self):
        """获取节点类型名称"""
        return self._node.get_type_name()
    
    def get_payload_key(self):
        """获取载荷键"""
        return self._node.get_pay_load_key_for_python()
    
    def set_payload_key(self, payload_key):
        """设置载荷键"""
        self._node.set_pay_load_key(payload_key)
    
    def set_attribute(self, name, value):
        """设置自定义属性"""
        return self._node.set_custom_attribute(name, str(value))
    
    def get_attribute(self, name):
        """获取自定义属性"""
        value = ""
        success = self._node.get_custom_attribute(name, value)
        return value if success else None
```

### 3.4 管道包装示例

```python
# custom_interchange/pipelines.py
import unreal

class CustomDemoObjectPipeline:
    """Python包装类，用于访问UCustomDemoObjectPipeline"""
    
    @staticmethod
    def create():
        """创建管道实例"""
        return unreal.CustomDemoObjectPipeline()
    
    @staticmethod
    def execute(file_paths, destination_path):
        """执行管道处理"""
        pipeline = unreal.CustomDemoObjectPipeline()
        return pipeline.execute_pipeline_from_python(file_paths, destination_path)
    
    @staticmethod
    def configure(config_options):
        """配置管道设置"""
        pipeline = unreal.CustomDemoObjectPipeline()
        # 将Python字典转换为Unreal TMap
        config_map = unreal.Map(str, str)
        for key, value in config_options.items():
            config_map.add(key, str(value))
        pipeline.configure_pipeline_from_python(config_map)
        return pipeline
```

## 4. 使用示例

### 4.1 基本导入示例

```python
import unreal
from custom_interchange import CustomFooTranslator

def import_custom_foo_file(file_path, destination_path="/Game/ImportedAssets/"):
    """导入自定义.foo格式文件"""
    # 检查文件是否存在
    import os
    if not os.path.exists(file_path):
        print(f"File not found: {file_path}")
        return False
    
    # 使用自定义翻译器导入
    result = CustomFooTranslator.translate_file(file_path, destination_path)
    
    if result:
        print(f"Successfully imported: {file_path}")
        return True
    else:
        print(f"Failed to import: {file_path}")
        return False

# 使用示例
success = import_custom_foo_file("C:/Assets/MyCustomFile.foo", "/Game/CustomAssets/")
```

### 4.2 高级配置示例

```python
import unreal
from custom_interchange import CustomDemoObjectPipeline

def import_with_custom_settings(file_paths, destination_path, settings=None):
    """使用自定义设置导入资产"""
    # 默认设置
    default_settings = {
        "bImportDemoObjects": "true",
        "bAssetSettingBoolean": "false",
        "AssetSettingFloat": "1.5"
    }
    
    # 合并用户设置
    if settings:
        default_settings.update(settings)
    
    # 配置管道
    pipeline = CustomDemoObjectPipeline.configure(default_settings)
    
    # 执行导入
    result = CustomDemoObjectPipeline.execute(file_paths, destination_path)
    
    return result

# 使用示例
files = ["C:/Assets/File1.foo", "C:/Assets/File2.foo"]
custom_settings = {
    "bAssetSettingBoolean": "true",
    "AssetSettingFloat": "2.5"
}

success = import_with_custom_settings(files, "/Game/CustomAssets/", custom_settings)
```

## 5. 实现路线图

### 5.1 阶段一：基础API暴露
1. 为关键类添加`UFUNCTION`和`UPROPERTY`标记
2. 实现Python友好的包装方法
3. 添加基本的错误处理和日志记录

### 5.2 阶段二：Python模块开发
1. 创建Python包装层结构
2. 实现核心功能包装类
3. 添加辅助工具和实用函数

### 5.3 阶段三：高级功能和集成
1. 实现批量处理和自动化工具
2. 添加与其他Python模块的集成
3. 开发示例脚本和文档

## 6. 结论

## 7. 高级Python集成应用

### 7.1 自动化工作流程集成

```python
# custom_interchange/automation.py
import unreal
import os
import json
from pathlib import Path
from typing import List, Dict, Optional
from .translators import CustomFooTranslator
from .pipelines import CustomDemoObjectPipeline

class InterchangeAutomationManager:
    """Interchange自动化管理器"""

    def __init__(self):
        self.config_file = "interchange_automation_config.json"
        self.load_configuration()

    def load_configuration(self):
        """加载自动化配置"""
        if os.path.exists(self.config_file):
            with open(self.config_file, 'r') as f:
                self.config = json.load(f)
        else:
            self.config = self.get_default_config()
            self.save_configuration()

    def save_configuration(self):
        """保存自动化配置"""
        with open(self.config_file, 'w') as f:
            json.dump(self.config, f, indent=2)

    def get_default_config(self) -> Dict:
        """获取默认配置"""
        return {
            "watch_folders": [],
            "import_rules": {
                "characters": {
                    "destination": "/Game/Characters/",
                    "pipeline_settings": {
                        "bImportDemoObjects": True,
                        "bAssetSettingBoolean": False,
                        "AssetSettingFloat": 1.0
                    }
                },
                "environments": {
                    "destination": "/Game/Environments/",
                    "pipeline_settings": {
                        "bImportDemoObjects": True,
                        "bAssetSettingBoolean": True,
                        "AssetSettingFloat": 2.0
                    }
                }
            },
            "quality_checks": {
                "enabled": True,
                "max_file_size_mb": 100,
                "allowed_extensions": [".foo", ".bar"]
            }
        }

    def add_watch_folder(self, folder_path: str, category: str):
        """添加监控文件夹"""
        watch_entry = {
            "path": folder_path,
            "category": category,
            "enabled": True
        }

        if watch_entry not in self.config["watch_folders"]:
            self.config["watch_folders"].append(watch_entry)
            self.save_configuration()

    def process_folder(self, folder_path: str, category: str) -> Dict:
        """处理文件夹中的所有文件"""
        results = {
            "processed": 0,
            "succeeded": 0,
            "failed": 0,
            "details": []
        }

        # 获取分类规则
        if category not in self.config["import_rules"]:
            print(f"Unknown category: {category}")
            return results

        rule = self.config["import_rules"][category]
        destination = rule["destination"]
        pipeline_settings = rule["pipeline_settings"]

        # 扫描文件
        files = self.scan_files(folder_path)
        results["processed"] = len(files)

        # 批量处理
        for file_path in files:
            try:
                success = self.process_single_file(file_path, destination, pipeline_settings)
                if success:
                    results["succeeded"] += 1
                    results["details"].append({"file": file_path, "status": "success"})
                else:
                    results["failed"] += 1
                    results["details"].append({"file": file_path, "status": "failed"})
            except Exception as e:
                results["failed"] += 1
                results["details"].append({"file": file_path, "status": "error", "message": str(e)})

        return results

    def scan_files(self, folder_path: str) -> List[str]:
        """扫描文件夹中的有效文件"""
        valid_files = []
        allowed_extensions = self.config["quality_checks"]["allowed_extensions"]
        max_size_mb = self.config["quality_checks"]["max_file_size_mb"]

        for root, dirs, files in os.walk(folder_path):
            for file in files:
                file_path = os.path.join(root, file)

                # 检查扩展名
                if any(file.lower().endswith(ext) for ext in allowed_extensions):
                    # 检查文件大小
                    file_size_mb = os.path.getsize(file_path) / (1024 * 1024)
                    if file_size_mb <= max_size_mb:
                        valid_files.append(file_path)
                    else:
                        print(f"File too large: {file_path} ({file_size_mb:.2f}MB)")

        return valid_files

    def process_single_file(self, file_path: str, destination: str, settings: Dict) -> bool:
        """处理单个文件"""
        # 质量检查
        if not self.perform_quality_check(file_path):
            return False

        # 配置管道
        pipeline = CustomDemoObjectPipeline.configure(settings)

        # 执行导入
        return CustomFooTranslator.translate_file(file_path, destination)

    def perform_quality_check(self, file_path: str) -> bool:
        """执行质量检查"""
        if not self.config["quality_checks"]["enabled"]:
            return True

        # 文件存在性检查
        if not os.path.exists(file_path):
            print(f"File not found: {file_path}")
            return False

        # 文件大小检查
        file_size_mb = os.path.getsize(file_path) / (1024 * 1024)
        max_size = self.config["quality_checks"]["max_file_size_mb"]
        if file_size_mb > max_size:
            print(f"File too large: {file_path} ({file_size_mb:.2f}MB > {max_size}MB)")
            return False

        # 文件格式检查
        allowed_extensions = self.config["quality_checks"]["allowed_extensions"]
        if not any(file_path.lower().endswith(ext) for ext in allowed_extensions):
            print(f"Unsupported file format: {file_path}")
            return False

        return True
```

### 7.2 实时监控系统

```python
# custom_interchange/monitoring.py
import time
import threading
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler
from .automation import InterchangeAutomationManager

class InterchangeFileHandler(FileSystemEventHandler):
    """文件系统事件处理器"""

    def __init__(self, automation_manager: InterchangeAutomationManager, category: str):
        self.automation_manager = automation_manager
        self.category = category
        self.processing_queue = []
        self.processing_lock = threading.Lock()

    def on_created(self, event):
        """文件创建事件"""
        if not event.is_directory:
            self.queue_file_for_processing(event.src_path)

    def on_modified(self, event):
        """文件修改事件"""
        if not event.is_directory:
            self.queue_file_for_processing(event.src_path)

    def queue_file_for_processing(self, file_path: str):
        """将文件加入处理队列"""
        with self.processing_lock:
            if file_path not in self.processing_queue:
                self.processing_queue.append(file_path)
                print(f"Queued for processing: {file_path}")

                # 启动处理线程
                threading.Thread(target=self.process_queued_files, daemon=True).start()

    def process_queued_files(self):
        """处理队列中的文件"""
        time.sleep(2)  # 等待文件写入完成

        with self.processing_lock:
            files_to_process = self.processing_queue.copy()
            self.processing_queue.clear()

        for file_path in files_to_process:
            try:
                # 获取导入规则
                if self.category in self.automation_manager.config["import_rules"]:
                    rule = self.automation_manager.config["import_rules"][self.category]
                    success = self.automation_manager.process_single_file(
                        file_path,
                        rule["destination"],
                        rule["pipeline_settings"]
                    )

                    if success:
                        print(f"✅ Successfully processed: {file_path}")
                    else:
                        print(f"❌ Failed to process: {file_path}")
                else:
                    print(f"⚠️ No rules for category: {self.category}")

            except Exception as e:
                print(f"❌ Error processing {file_path}: {e}")

class InterchangeMonitoringService:
    """Interchange监控服务"""

    def __init__(self):
        self.automation_manager = InterchangeAutomationManager()
        self.observers = {}
        self.is_running = False

    def start_monitoring(self):
        """开始监控"""
        if self.is_running:
            print("Monitoring is already running")
            return

        watch_folders = self.automation_manager.config["watch_folders"]

        for watch_entry in watch_folders:
            if watch_entry["enabled"]:
                self.add_watch_folder(
                    watch_entry["path"],
                    watch_entry["category"]
                )

        self.is_running = True
        print(f"Started monitoring {len(self.observers)} folders")

    def stop_monitoring(self):
        """停止监控"""
        for observer in self.observers.values():
            observer.stop()
            observer.join()

        self.observers.clear()
        self.is_running = False
        print("Stopped monitoring")

    def add_watch_folder(self, folder_path: str, category: str):
        """添加监控文件夹"""
        if not os.path.exists(folder_path):
            print(f"Watch folder does not exist: {folder_path}")
            return

        handler = InterchangeFileHandler(self.automation_manager, category)
        observer = Observer()
        observer.schedule(handler, folder_path, recursive=True)
        observer.start()

        self.observers[folder_path] = observer
        print(f"Added watch folder: {folder_path} (category: {category})")

    def remove_watch_folder(self, folder_path: str):
        """移除监控文件夹"""
        if folder_path in self.observers:
            self.observers[folder_path].stop()
            self.observers[folder_path].join()
            del self.observers[folder_path]
            print(f"Removed watch folder: {folder_path}")
```

### 7.3 批量处理和报告系统

```python
# custom_interchange/batch_processor.py
import concurrent.futures
import time
from datetime import datetime
from typing import List, Dict, Callable
from .automation import InterchangeAutomationManager

class BatchProcessor:
    """批量处理器"""

    def __init__(self, max_workers: int = 4):
        self.max_workers = max_workers
        self.automation_manager = InterchangeAutomationManager()
        self.progress_callback: Optional[Callable] = None

    def set_progress_callback(self, callback: Callable[[int, int, str], None]):
        """设置进度回调函数"""
        self.progress_callback = callback

    def process_files_parallel(self, file_list: List[str], category: str) -> Dict:
        """并行处理文件列表"""
        start_time = time.time()
        results = {
            "total_files": len(file_list),
            "processed": 0,
            "succeeded": 0,
            "failed": 0,
            "start_time": datetime.now().isoformat(),
            "details": []
        }

        # 获取处理规则
        if category not in self.automation_manager.config["import_rules"]:
            results["error"] = f"Unknown category: {category}"
            return results

        rule = self.automation_manager.config["import_rules"][category]

        # 并行处理
        with concurrent.futures.ThreadPoolExecutor(max_workers=self.max_workers) as executor:
            # 提交所有任务
            future_to_file = {
                executor.submit(
                    self.automation_manager.process_single_file,
                    file_path,
                    rule["destination"],
                    rule["pipeline_settings"]
                ): file_path for file_path in file_list
            }

            # 处理完成的任务
            for future in concurrent.futures.as_completed(future_to_file):
                file_path = future_to_file[future]
                results["processed"] += 1

                try:
                    success = future.result()
                    if success:
                        results["succeeded"] += 1
                        results["details"].append({
                            "file": file_path,
                            "status": "success",
                            "timestamp": datetime.now().isoformat()
                        })
                    else:
                        results["failed"] += 1
                        results["details"].append({
                            "file": file_path,
                            "status": "failed",
                            "timestamp": datetime.now().isoformat()
                        })

                except Exception as e:
                    results["failed"] += 1
                    results["details"].append({
                        "file": file_path,
                        "status": "error",
                        "error": str(e),
                        "timestamp": datetime.now().isoformat()
                    })

                # 更新进度
                if self.progress_callback:
                    self.progress_callback(
                        results["processed"],
                        results["total_files"],
                        file_path
                    )

        # 完成统计
        end_time = time.time()
        results["end_time"] = datetime.now().isoformat()
        results["duration_seconds"] = end_time - start_time
        results["files_per_second"] = results["processed"] / results["duration_seconds"] if results["duration_seconds"] > 0 else 0

        return results

    def generate_report(self, results: Dict, output_file: str = None) -> str:
        """生成处理报告"""
        report_lines = []
        report_lines.append("=" * 60)
        report_lines.append("INTERCHANGE BATCH PROCESSING REPORT")
        report_lines.append("=" * 60)
        report_lines.append(f"Start Time: {results['start_time']}")
        report_lines.append(f"End Time: {results['end_time']}")
        report_lines.append(f"Duration: {results['duration_seconds']:.2f} seconds")
        report_lines.append(f"Processing Speed: {results['files_per_second']:.2f} files/second")
        report_lines.append("")

        # 统计信息
        report_lines.append("SUMMARY:")
        report_lines.append(f"Total Files: {results['total_files']}")
        report_lines.append(f"Processed: {results['processed']}")
        report_lines.append(f"Succeeded: {results['succeeded']}")
        report_lines.append(f"Failed: {results['failed']}")

        if results['total_files'] > 0:
            success_rate = (results['succeeded'] / results['total_files']) * 100
            report_lines.append(f"Success Rate: {success_rate:.1f}%")

        report_lines.append("")

        # 失败详情
        failed_files = [detail for detail in results['details'] if detail['status'] != 'success']
        if failed_files:
            report_lines.append("FAILED FILES:")
            for detail in failed_files:
                report_lines.append(f"  ❌ {detail['file']}")
                if 'error' in detail:
                    report_lines.append(f"     Error: {detail['error']}")
            report_lines.append("")

        # 成功文件（可选，如果数量不多）
        successful_files = [detail for detail in results['details'] if detail['status'] == 'success']
        if successful_files and len(successful_files) <= 20:  # 只显示前20个成功文件
            report_lines.append("SUCCESSFUL FILES:")
            for detail in successful_files:
                report_lines.append(f"  ✅ {detail['file']}")
            report_lines.append("")

        report_lines.append("=" * 60)

        report_text = "\n".join(report_lines)

        # 输出到控制台
        print(report_text)

        # 保存到文件
        if output_file:
            with open(output_file, 'w', encoding='utf-8') as f:
                f.write(report_text)
            print(f"\nReport saved to: {output_file}")

        return report_text
```

## 8. 完整使用示例

### 8.1 自动化工作流程示例

```python
# example_automation_workflow.py
import os
from custom_interchange.automation import InterchangeAutomationManager
from custom_interchange.monitoring import InterchangeMonitoringService
from custom_interchange.batch_processor import BatchProcessor

def setup_game_asset_automation():
    """设置游戏资产自动化工作流程"""

    # 创建自动化管理器
    automation_manager = InterchangeAutomationManager()

    # 配置监控文件夹
    automation_manager.add_watch_folder("D:/GameAssets/Characters/", "characters")
    automation_manager.add_watch_folder("D:/GameAssets/Environments/", "environments")

    # 启动监控服务
    monitoring_service = InterchangeMonitoringService()
    monitoring_service.start_monitoring()

    print("🚀 Game asset automation is now running!")
    print("📁 Monitoring folders:")
    for watch_entry in automation_manager.config["watch_folders"]:
        print(f"   - {watch_entry['path']} ({watch_entry['category']})")

    return monitoring_service

def batch_import_existing_assets():
    """批量导入现有资产"""

    # 创建批量处理器
    batch_processor = BatchProcessor(max_workers=6)

    # 设置进度回调
    def progress_callback(processed, total, current_file):
        progress = (processed / total) * 100
        print(f"Progress: {progress:.1f}% ({processed}/{total}) - {os.path.basename(current_file)}")

    batch_processor.set_progress_callback(progress_callback)

    # 收集要处理的文件
    character_files = []
    for root, dirs, files in os.walk("D:/GameAssets/Characters/"):
        for file in files:
            if file.endswith('.foo'):
                character_files.append(os.path.join(root, file))

    print(f"Found {len(character_files)} character files to process")

    # 批量处理
    results = batch_processor.process_files_parallel(character_files, "characters")

    # 生成报告
    report_file = f"batch_import_report_{datetime.now().strftime('%Y%m%d_%H%M%S')}.txt"
    batch_processor.generate_report(results, report_file)

    return results

def main():
    """主函数"""
    print("🎮 Starting Interchange Automation System")

    try:
        # 1. 批量处理现有文件
        print("\n📦 Processing existing assets...")
        batch_results = batch_import_existing_assets()

        # 2. 启动实时监控
        print("\n👁️ Starting real-time monitoring...")
        monitoring_service = setup_game_asset_automation()

        # 3. 保持运行
        print("\n✅ System is running. Press Ctrl+C to stop.")
        try:
            while True:
                time.sleep(1)
        except KeyboardInterrupt:
            print("\n🛑 Stopping automation system...")
            monitoring_service.stop_monitoring()
            print("✅ System stopped successfully.")

    except Exception as e:
        print(f"❌ Error: {e}")
        return False

    return True

if __name__ == "__main__":
    import time
    from datetime import datetime
    success = main()
    exit(0 if success else 1)
```

## 9. 总结

将自定义Interchange插件API暴露给Python是完全可行的，并且可以显著增强插件的功能和灵活性。通过适当的C++修改和Python包装层，可以实现从Python脚本中完全控制自定义导入流程，为游戏开发团队提供强大的自动化工具。

### 主要优势：
1. **完全自动化**: 从文件监控到资产导入的全流程自动化
2. **高度可配置**: 通过JSON配置文件灵活调整导入规则
3. **并行处理**: 支持多线程批量处理，显著提升效率
4. **实时监控**: 文件系统监控，实现热重载功能
5. **详细报告**: 完整的处理日志和统计报告
6. **错误处理**: 健壮的错误处理和恢复机制
