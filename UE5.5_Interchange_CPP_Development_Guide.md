# Unreal Engine 5.5 Interchange C++ 开发完整指南

## 1. 开发环境准备

### 1.1 模块依赖配置
```cpp
// YourModule.Build.cs
public class YourCustomInterchange : ModuleRules
{
    public YourCustomInterchange(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InterchangeCore",           // 核心接口
            "InterchangeEngine",        // 引擎集成
            "InterchangeFactoryNodes",  // 工厂节点
            "InterchangeNodes",         // 标准节点
        });
        
        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "InterchangeCommonParser",  // 通用解析器
            "InterchangeMessages",      // 消息系统
            "Json",                     // JSON支持
        });
    }
}
```

### 1.2 必要的头文件包含
```cpp
// 核心头文件
#include "InterchangeTranslatorBase.h"
#include "InterchangePipelineBase.h"
#include "InterchangeFactoryBase.h"
#include "Nodes/InterchangeBaseNode.h"
#include "Nodes/InterchangeFactoryBaseNode.h"
#include "Nodes/InterchangeBaseNodeContainer.h"
#include "InterchangeManager.h"
#include "InterchangeSourceData.h"
```

## 2. 自定义翻译器开发

### 2.1 翻译器类定义
```cpp
// CustomAssetTranslator.h
#pragma once

#include "CoreMinimal.h"
#include "InterchangeTranslatorBase.h"
#include "Nodes/InterchangeBaseNodeContainer.h"
#include "CustomAssetPayloadInterface.h"
#include "CustomAssetTranslator.generated.h"

UCLASS(BlueprintType)
class YOURCUSTOMINTERCHANGE_API UCustomAssetTranslator : public UInterchangeTranslatorBase, 
                                                        public ICustomAssetPayloadInterface
{
    GENERATED_BODY()
    
public:
    // 必须实现的虚函数
    virtual TArray<FString> GetSupportedFormats() const override;
    virtual EInterchangeTranslatorAssetType GetSupportedAssetTypes() const override;
    virtual bool Translate(UInterchangeBaseNodeContainer& BaseNodeContainer) const override;
    
    // 载荷接口实现
    virtual TOptional<UE::Interchange::FCustomAssetData> GetCustomAssetPayloadData(const FString& PayloadKey) const override;
    
protected:
    // 辅助方法
    bool ValidateSourceFile() const;
    UCustomAssetNode* CreateAssetNode(UInterchangeBaseNodeContainer& Container, const FString& NodeUID) const;
};
```

### 2.2 翻译器实现最佳实践
```cpp
// CustomAssetTranslator.cpp
#include "CustomAssetTranslator.h"
#include "CustomAssetNode.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

// 控制台变量用于功能开关
static bool GInterchangeEnableCustomAssetImport = true;
static FAutoConsoleVariableRef CCvarInterchangeEnableCustomAssetImport(
    TEXT("Interchange.FeatureFlags.Import.CustomAsset"),
    GInterchangeEnableCustomAssetImport,
    TEXT("Whether CustomAsset support is enabled."),
    ECVF_Default);

TArray<FString> UCustomAssetTranslator::GetSupportedFormats() const
{
    TArray<FString> Formats;
    if (GInterchangeEnableCustomAssetImport || GIsAutomationTesting)
    {
        Formats.Add(TEXT("myformat;My Custom Format"));
        Formats.Add(TEXT("custom;Custom Asset Format"));
    }
    return Formats;
}

EInterchangeTranslatorAssetType UCustomAssetTranslator::GetSupportedAssetTypes() const
{
    return EInterchangeTranslatorAssetType::Textures | EInterchangeTranslatorAssetType::Materials;
}

bool UCustomAssetTranslator::Translate(UInterchangeBaseNodeContainer& BaseNodeContainer) const
{
    // 1. 验证源文件
    if (!ValidateSourceFile())
    {
        return false;
    }
    
    // 2. 解析文件内容
    FString FileContent;
    if (!FFileHelper::LoadFileToString(FileContent, *SourceData->GetFilename()))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load file: %s"), *SourceData->GetFilename());
        return false;
    }
    
    // 3. 创建节点
    FString NodeUID = FPaths::GetBaseFilename(SourceData->GetFilename());
    UCustomAssetNode* AssetNode = CreateAssetNode(BaseNodeContainer, NodeUID);
    if (!AssetNode)
    {
        return false;
    }
    
    // 4. 设置节点属性
    AssetNode->SetPayLoadKey(SourceData->GetFilename());
    AssetNode->SetCustomProperty(TEXT("FileSize"), FPlatformFileManager::Get().GetPlatformFile().FileSize(*SourceData->GetFilename()));
    
    // 5. 添加到容器
    BaseNodeContainer.AddNode(AssetNode);
    
    return true;
}

bool UCustomAssetTranslator::ValidateSourceFile() const
{
    FString Filename = SourceData->GetFilename();
    FPaths::NormalizeFilename(Filename);
    
    if (!FPaths::FileExists(Filename))
    {
        UE_LOG(LogTemp, Error, TEXT("Source file does not exist: %s"), *Filename);
        return false;
    }
    
    // 检查文件大小
    int64 FileSize = FPlatformFileManager::Get().GetPlatformFile().FileSize(*Filename);
    if (FileSize <= 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid file size: %lld"), FileSize);
        return false;
    }
    
    return true;
}

UCustomAssetNode* UCustomAssetTranslator::CreateAssetNode(UInterchangeBaseNodeContainer& Container, const FString& NodeUID) const
{
    UCustomAssetNode* AssetNode = NewObject<UCustomAssetNode>(&Container);
    if (!AssetNode)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create CustomAssetNode"));
        return nullptr;
    }
    
    FString DisplayLabel = FPaths::GetBaseFilename(SourceData->GetFilename());
    AssetNode->InitializeCustomAssetNode(NodeUID, DisplayLabel);
    
    return AssetNode;
}
```

## 3. 自定义节点开发

### 3.1 节点类定义
```cpp
// CustomAssetNode.h
#pragma once

#include "CoreMinimal.h"
#include "Nodes/InterchangeBaseNode.h"
#include "CustomAssetNode.generated.h"

// 静态数据定义
namespace UE::Interchange
{
    struct YOURCUSTOMINTERCHANGE_API FCustomAssetNodeStaticData : public FBaseNodeStaticData
    {
        static const FAttributeKey& PayLoadKey();
        static const FAttributeKey& CustomPropertyKey();
        static const FAttributeKey& AssetTypeKey();
    };
}

UCLASS(BlueprintType)
class YOURCUSTOMINTERCHANGE_API UCustomAssetNode : public UInterchangeBaseNode
{
    GENERATED_BODY()
    
public:
    UCustomAssetNode();
    
    // 节点初始化
    UFUNCTION(BlueprintCallable, Category = "Interchange | Node | CustomAsset")
    void InitializeCustomAssetNode(const FString& UniqueID, const FString& DisplayLabel);
    
    // 类型信息
    virtual FString GetTypeName() const override { return TEXT("CustomAssetNode"); }
    
    // 载荷键管理
    virtual const TOptional<FString> GetPayLoadKey() const;
    UFUNCTION(BlueprintCallable, Category = "Interchange | Node | CustomAsset")
    virtual void SetPayLoadKey(const FString& PayLoadKey);
    
    // 自定义属性
    UFUNCTION(BlueprintCallable, Category = "Interchange | Node | CustomAsset")
    bool GetCustomProperty(const FString& PropertyName, int64& OutValue) const;
    
    UFUNCTION(BlueprintCallable, Category = "Interchange | Node | CustomAsset")
    bool SetCustomProperty(const FString& PropertyName, int64 Value);
    
    // 资产类型
    UFUNCTION(BlueprintCallable, Category = "Interchange | Node | CustomAsset")
    bool GetAssetType(FString& OutAssetType) const;
    
    UFUNCTION(BlueprintCallable, Category = "Interchange | Node | CustomAsset")
    bool SetAssetType(const FString& AssetType);
};
```

### 3.2 节点实现
```cpp
// CustomAssetNode.cpp
#include "CustomAssetNode.h"

namespace UE::Interchange
{
    const FAttributeKey& FCustomAssetNodeStaticData::PayLoadKey()
    {
        static FAttributeKey AttributeKey(TEXT("__PayloadKey__"));
        return AttributeKey;
    }
    
    const FAttributeKey& FCustomAssetNodeStaticData::CustomPropertyKey()
    {
        static FAttributeKey AttributeKey(TEXT("__CustomProperty__"));
        return AttributeKey;
    }
    
    const FAttributeKey& FCustomAssetNodeStaticData::AssetTypeKey()
    {
        static FAttributeKey AttributeKey(TEXT("__AssetType__"));
        return AttributeKey;
    }
}

UCustomAssetNode::UCustomAssetNode()
{
    // 构造函数中的初始化
}

void UCustomAssetNode::InitializeCustomAssetNode(const FString& UniqueID, const FString& DisplayLabel)
{
    InitializeNode(UniqueID, DisplayLabel, EInterchangeNodeContainerType::TranslatedAsset);
}

const TOptional<FString> UCustomAssetNode::GetPayLoadKey() const
{
    FString PayloadKey;
    if (GetAttribute<FString>(UE::Interchange::FCustomAssetNodeStaticData::PayLoadKey(), PayloadKey))
    {
        return TOptional<FString>(PayloadKey);
    }
    return TOptional<FString>();
}

void UCustomAssetNode::SetPayLoadKey(const FString& PayLoadKey)
{
    SetAttribute<FString>(UE::Interchange::FCustomAssetNodeStaticData::PayLoadKey(), PayLoadKey);
}

bool UCustomAssetNode::GetCustomProperty(const FString& PropertyName, int64& OutValue) const
{
    FString AttributeKeyString = FString::Printf(TEXT("CustomProperty_%s"), *PropertyName);
    UE::Interchange::FAttributeKey AttributeKey(AttributeKeyString);
    return GetAttribute<int64>(AttributeKey, OutValue);
}

bool UCustomAssetNode::SetCustomProperty(const FString& PropertyName, int64 Value)
{
    FString AttributeKeyString = FString::Printf(TEXT("CustomProperty_%s"), *PropertyName);
    UE::Interchange::FAttributeKey AttributeKey(AttributeKeyString);
    return SetAttribute<int64>(AttributeKey, Value);
}
```

## 4. 自定义管道开发

### 4.1 管道类定义
```cpp
// CustomAssetPipeline.h
#pragma once

#include "CoreMinimal.h"
#include "InterchangePipelineBase.h"
#include "InterchangeSourceData.h"
#include "CustomAssetFactoryNode.h"
#include "CustomAssetPipeline.generated.h"

UCLASS(BlueprintType, editinlinenew)
class YOURCUSTOMINTERCHANGE_API UCustomAssetPipeline : public UInterchangePipelineBase
{
    GENERATED_BODY()
    
public:
    // 管道配置属性
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CustomAssets", 
              meta = (StandAlonePipelineProperty = "True", PipelineInternalEditionData = "True"))
    FString PipelineDisplayName = TEXT("Custom Asset Pipeline");
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CustomAssets")
    bool bImportCustomAssets = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CustomAssets", 
              meta = (EditCondition = "bImportCustomAssets"))
    bool bGenerateLightmapUVs = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CustomAssets", 
              meta = (EditCondition = "bImportCustomAssets", ClampMin = "0.0", ClampMax = "1.0"))
    float QualityScale = 1.0f;
    
    // 核心方法
    virtual void ExecutePipeline(UInterchangeBaseNodeContainer* InBaseNodeContainer, 
                                const TArray<UInterchangeSourceData*>& InSourceDatas, 
                                const FString& ContentBasePath) override;
    
#if WITH_EDITOR
    virtual void FilterPropertiesFromTranslatedData(UInterchangeBaseNodeContainer* InBaseNodeContainer) override;
#endif
    
protected:
    // 工厂节点创建
    UCustomAssetFactoryNode* CreateCustomAssetFactoryNode(const UCustomAssetNode* AssetNode);
    
    // 验证和处理
    bool ValidateAssetNode(const UCustomAssetNode* AssetNode) const;
    void ProcessAssetSettings(UCustomAssetFactoryNode* FactoryNode) const;
    
private:
    UPROPERTY()
    TObjectPtr<UInterchangeBaseNodeContainer> BaseNodeContainer;
    
    TArray<const UInterchangeSourceData*> SourceDatas;
    TArray<UCustomAssetNode*> CustomAssetNodes;
    TArray<UCustomAssetFactoryNode*> CustomAssetFactoryNodes;
};
```

### 4.2 管道实现
```cpp
// CustomAssetPipeline.cpp
#include "CustomAssetPipeline.h"
#include "CustomAssetNode.h"
#include "CustomAssetFactoryNode.h"

void UCustomAssetPipeline::ExecutePipeline(UInterchangeBaseNodeContainer* InBaseNodeContainer, 
                                          const TArray<UInterchangeSourceData*>& InSourceDatas, 
                                          const FString& ContentBasePath)
{
    if (!InBaseNodeContainer)
    {
        UE_LOG(LogTemp, Warning, TEXT("UCustomAssetPipeline: BaseNodeContainer is null"));
        return;
    }
    
    // 存储引用
    BaseNodeContainer = InBaseNodeContainer;
    SourceDatas.Empty(InSourceDatas.Num());
    for (const UInterchangeSourceData* SourceData : InSourceDatas)
    {
        SourceDatas.Add(SourceData);
    }
    
    // 收集相关节点
    BaseNodeContainer->IterateNodes([this](const FString& NodeUid, UInterchangeBaseNode* Node)
    {
        if (Node->GetNodeContainerType() == EInterchangeNodeContainerType::TranslatedAsset)
        {
            if (UCustomAssetNode* CustomAssetNode = Cast<UCustomAssetNode>(Node))
            {
                if (ValidateAssetNode(CustomAssetNode))
                {
                    CustomAssetNodes.Add(CustomAssetNode);
                }
            }
        }
    });
    
    // 创建工厂节点
    if (bImportCustomAssets)
    {
        for (UCustomAssetNode* AssetNode : CustomAssetNodes)
        {
            UCustomAssetFactoryNode* FactoryNode = CreateCustomAssetFactoryNode(AssetNode);
            if (FactoryNode)
            {
                ProcessAssetSettings(FactoryNode);
                CustomAssetFactoryNodes.Add(FactoryNode);
            }
        }
    }
}

bool UCustomAssetPipeline::ValidateAssetNode(const UCustomAssetNode* AssetNode) const
{
    if (!AssetNode)
    {
        return false;
    }
    
    // 检查载荷键
    TOptional<FString> PayloadKey = AssetNode->GetPayLoadKey();
    if (!PayloadKey.IsSet() || PayloadKey->IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("CustomAssetNode has invalid payload key"));
        return false;
    }
    
    // 检查资产类型
    FString AssetType;
    if (!AssetNode->GetAssetType(AssetType) || AssetType.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("CustomAssetNode has invalid asset type"));
        return false;
    }
    
    return true;
}

void UCustomAssetPipeline::ProcessAssetSettings(UCustomAssetFactoryNode* FactoryNode) const
{
    if (!FactoryNode)
    {
        return;
    }
    
    // 应用管道设置到工厂节点
    constexpr bool bAddApplyDelegate = true;
    FactoryNode->SetCustomGenerateLightmapUVs(bGenerateLightmapUVs, bAddApplyDelegate);
    FactoryNode->SetCustomQualityScale(QualityScale, bAddApplyDelegate);
}
```

## 5. 自定义工厂开发

### 5.1 工厂类定义
```cpp
// CustomAssetFactory.h
#pragma once

#include "CoreMinimal.h"
#include "InterchangeFactoryBase.h"
#include "CustomAssetPayloadData.h"
#include "CustomAssetFactory.generated.h"

UCLASS(BlueprintType)
class YOURCUSTOMINTERCHANGE_API UCustomAssetFactory : public UInterchangeFactoryBase
{
    GENERATED_BODY()
    
public:
    // 工厂基础信息
    virtual UClass* GetFactoryClass() const override;
    virtual EInterchangeFactoryAssetType GetFactoryAssetType() override;
    
    // 三阶段导入流程
    virtual FImportAssetResult BeginImportAsset_GameThread(const FImportAssetObjectParams& Arguments) override;
    virtual FImportAssetResult ImportAsset_Async(const FImportAssetObjectParams& Arguments) override;
    virtual void SetupObject_GameThread(const FSetupObjectParams& Arguments) override;
    
    // 源文件管理
    virtual bool GetSourceFilenames(const UObject* Object, TArray<FString>& OutSourceFilenames) const override;
    virtual bool SetSourceFilename(const UObject* Object, const FString& SourceFilename, int32 SourceIndex) const override;
    
protected:
    // 辅助方法
    bool ValidateFactoryNode(const UCustomAssetFactoryNode* FactoryNode) const;
    void ApplyCustomSettings(UCustomAsset* Asset, const UCustomAssetFactoryNode* FactoryNode) const;
    
private:
    bool bSkipImport = false;
};
```

### 5.2 工厂实现关键方法
```cpp
// CustomAssetFactory.cpp
#include "CustomAssetFactory.h"
#include "CustomAsset.h"
#include "CustomAssetFactoryNode.h"
#include "CustomAssetPayloadInterface.h"

UClass* UCustomAssetFactory::GetFactoryClass() const
{
    return UCustomAsset::StaticClass();
}

EInterchangeFactoryAssetType UCustomAssetFactory::GetFactoryAssetType()
{
    return EInterchangeFactoryAssetType::Textures;
}

FImportAssetResult UCustomAssetFactory::BeginImportAsset_GameThread(const FImportAssetObjectParams& Arguments)
{
    FImportAssetResult ImportAssetResult;
    
    // 验证工厂节点
    UCustomAssetFactoryNode* FactoryNode = Cast<UCustomAssetFactoryNode>(Arguments.FactoryNode);
    if (!ValidateFactoryNode(FactoryNode))
    {
        ImportAssetResult.bIsFactorySkipAsset = true;
        return ImportAssetResult;
    }
    
    // 创建或获取资产
    UCustomAsset* CustomAsset = nullptr;
    if (Arguments.ReplacedAsset)
    {
        CustomAsset = Cast<UCustomAsset>(Arguments.ReplacedAsset);
    }
    else
    {
        CustomAsset = NewObject<UCustomAsset>(Arguments.Parent, Arguments.AssetClass, 
                                             *Arguments.AssetName, RF_Public | RF_Standalone);
    }
    
    if (!CustomAsset)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create CustomAsset"));
        return ImportAssetResult;
    }
    
    ImportAssetResult.ImportedObject = CustomAsset;
    return ImportAssetResult;
}

FImportAssetResult UCustomAssetFactory::ImportAsset_Async(const FImportAssetObjectParams& Arguments)
{
    FImportAssetResult ImportAssetResult;
    ImportAssetResult.bIsFactorySkipAsset = bSkipImport;
    
    if (bSkipImport)
    {
        return ImportAssetResult;
    }
    
    // 获取载荷接口
    const ICustomAssetPayloadInterface* PayloadInterface = Cast<ICustomAssetPayloadInterface>(Arguments.Translator);
    if (!PayloadInterface)
    {
        UE_LOG(LogTemp, Error, TEXT("Translator does not implement ICustomAssetPayloadInterface"));
        return ImportAssetResult;
    }
    
    // 获取载荷数据
    UCustomAssetFactoryNode* FactoryNode = Cast<UCustomAssetFactoryNode>(Arguments.FactoryNode);
    TOptional<FString> PayloadKey = FactoryNode->GetPayLoadKey();
    if (!PayloadKey.IsSet())
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid payload key"));
        return ImportAssetResult;
    }
    
    TOptional<UE::Interchange::FCustomAssetData> PayloadData = PayloadInterface->GetCustomAssetPayloadData(PayloadKey.GetValue());
    if (!PayloadData.IsSet())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get payload data"));
        return ImportAssetResult;
    }
    
    // 处理载荷数据
    UCustomAsset* CustomAsset = Cast<UCustomAsset>(Arguments.ImportedObject);
    if (CustomAsset)
    {
        // 在这里处理载荷数据，例如：
        CustomAsset->SetSourceData(PayloadData->RawData);
        CustomAsset->SetMetadata(PayloadData->Metadata);
    }
    
    return ImportAssetResult;
}
```

## 6. 模块注册与集成

### 6.1 模块类实现
```cpp
// YourCustomInterchangeModule.cpp
#include "YourCustomInterchangeModule.h"
#include "InterchangeManager.h"
#include "CustomAssetTranslator.h"
#include "CustomAssetFactory.h"

class FYourCustomInterchangeModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE(FYourCustomInterchangeModule, YourCustomInterchange)

void FYourCustomInterchangeModule::StartupModule()
{
    auto RegisterItems = []()
    {
        UInterchangeManager& InterchangeManager = UInterchangeManager::GetInterchangeManager();
        
        // 注册翻译器
        InterchangeManager.RegisterTranslator(UCustomAssetTranslator::StaticClass());
        
        // 注册工厂
        InterchangeManager.RegisterFactory(UCustomAssetFactory::StaticClass());
        
        UE_LOG(LogTemp, Log, TEXT("YourCustomInterchange module registered successfully"));
    };
    
    if (GEngine)
    {
        RegisterItems();
    }
    else
    {
        FCoreDelegates::OnPostEngineInit.AddLambda(RegisterItems);
    }
}

void FYourCustomInterchangeModule::ShutdownModule()
{
    // 清理工作
}
```

## 7. 最佳实践总结

### 7.1 性能优化
1. **异步处理**: 重载荷操作放在 `ImportAsset_Async` 中
2. **内存管理**: 使用 `TObjectPtr` 和智能指针
3. **延迟加载**: 载荷数据按需获取
4. **批处理**: 尽可能批量处理节点

### 7.2 错误处理
1. **验证输入**: 始终验证输入参数
2. **结构化日志**: 使用专用日志类别
3. **错误恢复**: 提供合理的默认值
4. **用户反馈**: 使用 Interchange 消息系统

### 7.3 代码组织
1. **模块化**: 按功能分离代码
2. **接口设计**: 使用纯虚函数定义接口
3. **属性系统**: 利用 UE 的属性系统
4. **蓝图支持**: 适当暴露蓝图接口

## 8. 高级开发技巧

### 8.1 自定义属性宏系统
```cpp
// 简化属性定义的宏系统
#define DECLARE_INTERCHANGE_ATTRIBUTE(AttributeName, AttributeType) \
private: \
    static const UE::Interchange::FAttributeKey& Get##AttributeName##Key() \
    { \
        static UE::Interchange::FAttributeKey Key(TEXT(#AttributeName)); \
        return Key; \
    } \
public: \
    bool Get##AttributeName(AttributeType& OutValue) const \
    { \
        return GetAttribute<AttributeType>(Get##AttributeName##Key(), OutValue); \
    } \
    bool Set##AttributeName(const AttributeType& Value) \
    { \
        return SetAttribute<AttributeType>(Get##AttributeName##Key(), Value); \
    }

// 使用示例
class UMyCustomNode : public UInterchangeBaseNode
{
    GENERATED_BODY()

    DECLARE_INTERCHANGE_ATTRIBUTE(Quality, float)
    DECLARE_INTERCHANGE_ATTRIBUTE(EnableLOD, bool)
    DECLARE_INTERCHANGE_ATTRIBUTE(MaterialPath, FString)
};
```

### 8.2 载荷数据缓存机制
```cpp
// 载荷缓存管理器
class FPayloadCacheManager
{
private:
    static TMap<FString, TSharedPtr<UE::Interchange::FCustomAssetData>> PayloadCache;
    static FCriticalSection CacheMutex;

public:
    static TOptional<UE::Interchange::FCustomAssetData> GetCachedPayload(const FString& PayloadKey)
    {
        FScopeLock Lock(&CacheMutex);
        if (TSharedPtr<UE::Interchange::FCustomAssetData>* Found = PayloadCache.Find(PayloadKey))
        {
            return **Found;
        }
        return TOptional<UE::Interchange::FCustomAssetData>();
    }

    static void CachePayload(const FString& PayloadKey, const UE::Interchange::FCustomAssetData& Data)
    {
        FScopeLock Lock(&CacheMutex);
        PayloadCache.Add(PayloadKey, MakeShared<UE::Interchange::FCustomAssetData>(Data));
    }

    static void ClearCache()
    {
        FScopeLock Lock(&CacheMutex);
        PayloadCache.Empty();
    }
};
```

### 8.3 多线程安全的翻译器
```cpp
class UThreadSafeTranslator : public UInterchangeTranslatorBase
{
private:
    mutable FCriticalSection TranslationMutex;
    mutable TMap<FString, FDateTime> FileModificationTimes;

public:
    virtual bool Translate(UInterchangeBaseNodeContainer& BaseNodeContainer) const override
    {
        FScopeLock Lock(&TranslationMutex);

        // 检查文件修改时间
        FString Filename = SourceData->GetFilename();
        FDateTime ModTime = FPlatformFileManager::Get().GetPlatformFile().GetTimeStamp(*Filename);

        if (FileModificationTimes.Contains(Filename))
        {
            if (FileModificationTimes[Filename] == ModTime)
            {
                UE_LOG(LogTemp, Log, TEXT("File unchanged, skipping translation: %s"), *Filename);
                return true; // 文件未修改，跳过翻译
            }
        }

        FileModificationTimes.Add(Filename, ModTime);

        // 执行实际翻译
        return PerformTranslation(BaseNodeContainer);
    }

protected:
    virtual bool PerformTranslation(UInterchangeBaseNodeContainer& BaseNodeContainer) const
    {
        // 实际的翻译逻辑
        return true;
    }
};
```

## 9. 调试和测试

### 9.1 调试辅助工具
```cpp
// 调试信息收集器
class FInterchangeDebugHelper
{
public:
    static void DumpNodeContainer(const UInterchangeBaseNodeContainer* Container)
    {
        if (!Container)
        {
            UE_LOG(LogTemp, Warning, TEXT("NodeContainer is null"));
            return;
        }

        UE_LOG(LogTemp, Log, TEXT("=== Node Container Debug Info ==="));

        Container->IterateNodes([](const FString& NodeUid, UInterchangeBaseNode* Node)
        {
            UE_LOG(LogTemp, Log, TEXT("Node: %s | Type: %s | Container: %d"),
                   *NodeUid,
                   *Node->GetTypeName(),
                   (int32)Node->GetNodeContainerType());
        });
    }

    static void DumpNodeAttributes(const UInterchangeBaseNode* Node)
    {
        if (!Node)
        {
            return;
        }

        UE_LOG(LogTemp, Log, TEXT("=== Node Attributes: %s ==="), *Node->GetUniqueID());

        // 这里需要访问节点的属性存储，具体实现依赖于UE的内部API
        // 在实际项目中，可以通过反射或友元类来访问私有成员
    }
};

// 在翻译器中使用
bool UCustomAssetTranslator::Translate(UInterchangeBaseNodeContainer& BaseNodeContainer) const
{
    bool bResult = PerformTranslation(BaseNodeContainer);

#if !UE_BUILD_SHIPPING
    if (CVarEnableInterchangeDebug.GetValueOnAnyThread())
    {
        FInterchangeDebugHelper::DumpNodeContainer(&BaseNodeContainer);
    }
#endif

    return bResult;
}
```

### 9.2 单元测试框架
```cpp
// 测试基类
class FInterchangeTestBase
{
protected:
    TObjectPtr<UInterchangeBaseNodeContainer> TestContainer;
    TObjectPtr<UCustomAssetTranslator> TestTranslator;

public:
    virtual void SetUp()
    {
        TestContainer = NewObject<UInterchangeBaseNodeContainer>();
        TestTranslator = NewObject<UCustomAssetTranslator>();
    }

    virtual void TearDown()
    {
        TestContainer = nullptr;
        TestTranslator = nullptr;
    }

    bool CreateTestFile(const FString& Filename, const FString& Content)
    {
        return FFileHelper::SaveStringToFile(Content, *Filename);
    }

    void CleanupTestFile(const FString& Filename)
    {
        FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*Filename);
    }
};

// 具体测试用例
class FCustomAssetTranslatorTest : public FInterchangeTestBase
{
public:
    void TestBasicTranslation()
    {
        // 创建测试文件
        FString TestFilename = FPaths::ProjectIntermediateDir() / TEXT("test_asset.myformat");
        FString TestContent = TEXT("{\"type\":\"test\",\"data\":\"sample\"}");

        if (!CreateTestFile(TestFilename, TestContent))
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to create test file"));
            return;
        }

        // 设置源数据
        UInterchangeSourceData* SourceData = NewObject<UInterchangeSourceData>();
        SourceData->SetFilename(TestFilename);
        TestTranslator->SetSourceData(SourceData);

        // 执行翻译
        bool bResult = TestTranslator->Translate(*TestContainer);

        // 验证结果
        if (bResult)
        {
            TArray<FString> NodeUIDs;
            TestContainer->GetNodes(UCustomAssetNode::StaticClass(), NodeUIDs);

            if (NodeUIDs.Num() == 1)
            {
                UE_LOG(LogTemp, Log, TEXT("Translation test PASSED"));
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Translation test FAILED: Expected 1 node, got %d"), NodeUIDs.Num());
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Translation test FAILED: Translation returned false"));
        }

        // 清理
        CleanupTestFile(TestFilename);
    }
};
```

## 10. 性能分析和优化

### 10.1 性能监控
```cpp
// 性能计时器
class FScopedInterchangeTimer
{
private:
    FString OperationName;
    double StartTime;

public:
    FScopedInterchangeTimer(const FString& InOperationName)
        : OperationName(InOperationName)
        , StartTime(FPlatformTime::Seconds())
    {
    }

    ~FScopedInterchangeTimer()
    {
        double ElapsedTime = FPlatformTime::Seconds() - StartTime;
        UE_LOG(LogTemp, Log, TEXT("Interchange Operation '%s' took %.3f seconds"), *OperationName, ElapsedTime);
    }
};

// 使用宏简化
#define INTERCHANGE_SCOPED_TIMER(Name) FScopedInterchangeTimer Timer(Name)

// 在关键方法中使用
bool UCustomAssetTranslator::Translate(UInterchangeBaseNodeContainer& BaseNodeContainer) const
{
    INTERCHANGE_SCOPED_TIMER(TEXT("CustomAssetTranslator::Translate"));

    // 翻译逻辑
    return true;
}
```

### 10.2 内存使用优化
```cpp
// 内存池管理器
class FInterchangeMemoryPool
{
private:
    static TArray<void*> MemoryBlocks;
    static FCriticalSection PoolMutex;
    static constexpr size_t BlockSize = 1024 * 1024; // 1MB blocks

public:
    static void* Allocate(size_t Size)
    {
        if (Size > BlockSize)
        {
            return FMemory::Malloc(Size); // 大块内存直接分配
        }

        FScopeLock Lock(&PoolMutex);

        // 从池中分配
        if (MemoryBlocks.Num() > 0)
        {
            void* Block = MemoryBlocks.Pop();
            return Block;
        }

        return FMemory::Malloc(BlockSize);
    }

    static void Deallocate(void* Ptr, size_t Size)
    {
        if (Size > BlockSize)
        {
            FMemory::Free(Ptr);
            return;
        }

        FScopeLock Lock(&PoolMutex);
        MemoryBlocks.Add(Ptr);
    }

    static void Cleanup()
    {
        FScopeLock Lock(&PoolMutex);
        for (void* Block : MemoryBlocks)
        {
            FMemory::Free(Block);
        }
        MemoryBlocks.Empty();
    }
};
```

这个开发指南提供了完整的 C++ 开发框架和最佳实践，包括高级技巧、调试工具和性能优化策略。
