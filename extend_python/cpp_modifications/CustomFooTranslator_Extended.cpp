// Copyright Epic Games, Inc. All Rights Reserved.
// Extended version with Python API support

#include "CustomFooTranslator_Extended.h"
#include "InterchangeManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "CustomDemoObjectNode.h"

// Console variable for enabling Python features
static bool GInterchangeEnablePythonExtensions = true;
static FAutoConsoleVariableRef CCvarInterchangeEnablePythonExtensions(
    TEXT("Interchange.FeatureFlags.EnablePythonExtensions"),
    GInterchangeEnablePythonExtensions,
    TEXT("Whether Python extensions for Interchange are enabled."),
    ECVF_Default);

// 构造函数 - 初始化翻译器扩展实例
UCustomFooTranslatorExtended::UCustomFooTranslatorExtended()
    : TranslationCount(0)
    , SuccessfulTranslations(0)
    , FailedTranslations(0)
    , TotalTranslationTime(0.0)
{
    InitializeDefaultOptions(); // 初始化默认选项
}

// 获取支持的文件格式列表
TArray<FString> UCustomFooTranslatorExtended::GetSupportedFormats() const
{
    TArray<FString> Formats;
    if (GInterchangeEnableCustomFooImport || GIsAutomationTesting)
    {
        Formats.Add(TEXT("foo;FOO my game format")); // 添加.foo格式支持
    }
    return Formats;
}

// 获取支持的资产类型
EInterchangeTranslatorAssetType UCustomFooTranslatorExtended::GetSupportedAssetTypes() const
{
    return EInterchangeTranslatorAssetType::None; // 返回无特定类型限制
}

// 执行文件翻译的主要方法
bool UCustomFooTranslatorExtended::Translate(UInterchangeBaseNodeContainer& BaseNodeContainer) const
{
    double StartTime = FPlatformTime::Seconds(); // 记录开始时间

    FString Filename = SourceData->GetFilename(); // 获取源文件名
    bool bResult = ExecuteTranslationInternal(BaseNodeContainer, Filename); // 执行内部翻译逻辑

    double TranslationTime = FPlatformTime::Seconds() - StartTime; // 计算翻译耗时
    UpdateStatistics(bResult, TranslationTime); // 更新统计信息

    if (bResult)
    {
        LogTranslationEvent(TEXT("Info"), TEXT("Translation completed successfully"), Filename); // 记录成功日志
    }
    else
    {
        LogTranslationEvent(TEXT("Error"), TEXT("Translation failed"), Filename); // 记录失败日志
    }

    return bResult;
}

// 获取演示对象的载荷数据
TOptional<UE::Interchange::FCustomDemoObjectData> UCustomFooTranslatorExtended::GetDemoObjectPayloadData(const FString& PayloadKey) const
{
    UE::Interchange::FCustomDemoObjectData CustomDemoObjectData;

    // 尝试从文件加载载荷数据
    if (!FFileHelper::LoadFileToString(CustomDemoObjectData.FormatJsonData, *PayloadKey))
    {
        LogTranslationEvent(TEXT("Error"), TEXT("Failed to load payload data"), PayloadKey); // 记录加载失败日志
        return TOptional<UE::Interchange::FCustomDemoObjectData>(); // 返回空值
    }

    return MoveTemp(CustomDemoObjectData); // 返回载荷数据
}

// Python可访问方法的实现

// 为Python提供的获取支持格式方法
TArray<FString> UCustomFooTranslatorExtended::GetSupportedFormatsForPython() const
{
    return GetSupportedFormats(); // 直接调用基础方法
}

// 从Python调用的文件翻译方法
bool UCustomFooTranslatorExtended::TranslateFileFromPython(const FString& FilePath, const FString& DestinationPath)
{
    // 检查Python扩展是否启用
    if (!GInterchangeEnablePythonExtensions)
    {
        LogTranslationEvent(TEXT("Warning"), TEXT("Python extensions are disabled")); // Python扩展被禁用
        return false;
    }

    // 验证输入文件
    FString ErrorMessage;
    if (!ValidateFileForTranslation(FilePath, ErrorMessage))
    {
        LogTranslationEvent(TEXT("Error"), ErrorMessage, FilePath); // 记录验证错误
        return false;
    }

    // 创建源数据对象
    UInterchangeSourceData* PythonSourceData = UInterchangeManager::CreateSourceData(FilePath);
    if (!PythonSourceData)
    {
        LogTranslationEvent(TEXT("Error"), TEXT("Failed to create source data"), FilePath); // 创建源数据失败
        return false;
    }

    // 临时设置源数据
    const UInterchangeSourceData* OriginalSourceData = SourceData;
    SourceData = PythonSourceData;

    // 创建节点容器
    UInterchangeBaseNodeContainer* Container = CreateNodeContainerForPython();
    if (!Container)
    {
        SourceData = OriginalSourceData; // 恢复原始源数据
        LogTranslationEvent(TEXT("Error"), TEXT("Failed to create node container"), FilePath); // 创建容器失败
        return false;
    }

    // 执行翻译
    bool bResult = Translate(*Container);

    // 恢复原始源数据
    SourceData = OriginalSourceData;

    if (bResult)
    {
        // 使用Interchange管理器导入资产
        UInterchangeManager& InterchangeManager = UInterchangeManager::GetInterchangeManager();
        bResult = InterchangeManager.ImportAsset(DestinationPath, PythonSourceData) != nullptr;
    }

    return bResult;
}

// 将载荷数据转换为JSON字符串格式
FString UCustomFooTranslatorExtended::GetPayloadDataAsJson(const FString& PayloadKey) const
{
    TOptional<UE::Interchange::FCustomDemoObjectData> PayloadData = GetDemoObjectPayloadData(PayloadKey);

    // 如果载荷数据不存在，返回空JSON对象
    if (!PayloadData.IsSet())
    {
        return TEXT("{}");
    }

    // 创建JSON对象
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetStringField(TEXT("FormatJsonData"), PayloadData->FormatJsonData); // 设置格式数据
    JsonObject->SetStringField(TEXT("PayloadKey"), PayloadKey); // 设置载荷键
    JsonObject->SetStringField(TEXT("Timestamp"), FDateTime::Now().ToString()); // 设置时间戳

    // 序列化为字符串
    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    return OutputString;
}

// 检查文件是否可以被此翻译器处理
bool UCustomFooTranslatorExtended::CanTranslateFile(const FString& FilePath) const
{
    // 检查文件扩展名
    FString Extension = FPaths::GetExtension(FilePath).ToLower();
    TArray<FString> SupportedFormats = GetSupportedFormats(); // 获取支持的格式列表

    // 遍历支持的格式
    for (const FString& Format : SupportedFormats)
    {
        FString FormatExtension;
        if (Format.Split(TEXT(";"), &FormatExtension, nullptr)) // 分割格式字符串
        {
            if (Extension == FormatExtension.ToLower()) // 比较扩展名
            {
                return true; // 找到匹配的格式
            }
        }
    }

    return false; // 不支持此文件格式
}

// 获取翻译器信息
TMap<FString, FString> UCustomFooTranslatorExtended::GetTranslatorInfo() const
{
    TMap<FString, FString> Info;
    Info.Add(TEXT("Name"), TEXT("Custom Foo Translator Extended")); // 翻译器名称
    Info.Add(TEXT("Version"), TEXT("1.0.0")); // 版本号
    Info.Add(TEXT("SupportedFormats"), FString::Join(GetSupportedFormats(), TEXT(", "))); // 支持的格式
    Info.Add(TEXT("PythonEnabled"), GInterchangeEnablePythonExtensions ? TEXT("true") : TEXT("false")); // Python是否启用
    Info.Add(TEXT("TranslationCount"), FString::FromInt(TranslationCount)); // 翻译次数
    Info.Add(TEXT("SuccessRate"), TranslationCount > 0 ? FString::Printf(TEXT("%.2f%%"), (float)SuccessfulTranslations / TranslationCount * 100.0f) : TEXT("N/A")); // 成功率

    return Info;
}

// 验证文件是否可以进行翻译
bool UCustomFooTranslatorExtended::ValidateFileForTranslation(const FString& FilePath, FString& OutErrorMessage) const
{
    // 检查文件是否存在
    if (!FPaths::FileExists(FilePath))
    {
        OutErrorMessage = FString::Printf(TEXT("File does not exist: %s"), *FilePath); // 文件不存在
        return false;
    }

    // 检查文件是否可以被翻译
    if (!CanTranslateFile(FilePath))
    {
        OutErrorMessage = FString::Printf(TEXT("File format not supported: %s"), *FPaths::GetExtension(FilePath)); // 不支持的文件格式
        return false;
    }

    // 检查文件大小
    int64 FileSize = FPlatformFileManager::Get().GetPlatformFile().FileSize(*FilePath);
    if (FileSize <= 0)
    {
        OutErrorMessage = FString::Printf(TEXT("Invalid file size: %lld"), FileSize); // 无效的文件大小
        return false;
    }

    // 检查文件是否可读
    FString TestContent;
    if (!FFileHelper::LoadFileToString(TestContent, *FilePath))
    {
        OutErrorMessage = FString::Printf(TEXT("Cannot read file: %s"), *FilePath); // 无法读取文件
        return false;
    }

    OutErrorMessage = TEXT(""); // 清空错误消息
    return true; // 验证通过
}

// 设置翻译选项
void UCustomFooTranslatorExtended::SetTranslationOptions(const TMap<FString, FString>& Options)
{
    // 遍历并添加所有选项
    for (const auto& Option : Options)
    {
        TranslationOptions.Add(Option.Key, Option.Value); // 添加或更新选项
    }

    LogTranslationEvent(TEXT("Info"), FString::Printf(TEXT("Updated %d translation options"), Options.Num())); // 记录更新日志
}

// 获取当前翻译选项
TMap<FString, FString> UCustomFooTranslatorExtended::GetTranslationOptions() const
{
    return TranslationOptions; // 返回当前选项映射
}

// 重置翻译选项为默认值
void UCustomFooTranslatorExtended::ResetTranslationOptions()
{
    TranslationOptions.Empty(); // 清空现有选项
    InitializeDefaultOptions(); // 重新初始化默认选项
    LogTranslationEvent(TEXT("Info"), TEXT("Reset translation options to defaults")); // 记录重置日志
}

// 获取翻译统计信息
TMap<FString, FString> UCustomFooTranslatorExtended::GetTranslationStatistics() const
{
    TMap<FString, FString> Stats;
    Stats.Add(TEXT("TotalTranslations"), FString::FromInt(TranslationCount)); // 总翻译次数
    Stats.Add(TEXT("SuccessfulTranslations"), FString::FromInt(SuccessfulTranslations)); // 成功翻译次数
    Stats.Add(TEXT("FailedTranslations"), FString::FromInt(FailedTranslations)); // 失败翻译次数
    Stats.Add(TEXT("SuccessRate"), TranslationCount > 0 ? FString::Printf(TEXT("%.2f%%"), (float)SuccessfulTranslations / TranslationCount * 100.0f) : TEXT("0%")); // 成功率
    Stats.Add(TEXT("AverageTranslationTime"), TranslationCount > 0 ? FString::Printf(TEXT("%.3f seconds"), TotalTranslationTime / TranslationCount) : TEXT("0 seconds")); // 平均翻译时间
    Stats.Add(TEXT("TotalTranslationTime"), FString::Printf(TEXT("%.3f seconds"), TotalTranslationTime)); // 总翻译时间

    return Stats;
}

// 受保护的辅助方法

// 为Python创建节点容器
UInterchangeBaseNodeContainer* UCustomFooTranslatorExtended::CreateNodeContainerForPython() const
{
    return NewObject<UInterchangeBaseNodeContainer>(); // 创建新的节点容器实例
}

// 执行内部翻译逻辑
bool UCustomFooTranslatorExtended::ExecuteTranslationInternal(UInterchangeBaseNodeContainer& Container, const FString& FilePath) const
{
    // 规范化文件名
    FString NormalizedFilename = FilePath;
    FPaths::NormalizeFilename(NormalizedFilename); // 标准化路径格式

    // 检查文件是否存在
    if (!FPaths::FileExists(NormalizedFilename))
    {
        return false; // 文件不存在
    }

    // 创建自定义演示对象节点
    FString DisplayLabel = FPaths::GetBaseFilename(NormalizedFilename); // 获取文件基础名作为显示标签
    FString NodeUID(NormalizedFilename); // 使用文件路径作为节点唯一ID
    UCustomDemoObjectNode* CustomDemoObjectNode = NewObject<UCustomDemoObjectNode>(&Container);

    if (!CustomDemoObjectNode)
    {
        return false; // 创建节点失败
    }

    // 初始化节点
    CustomDemoObjectNode->InitializeCustomDemoObjectNode(NodeUID, DisplayLabel);
    CustomDemoObjectNode->SetPayLoadKey(NormalizedFilename); // 设置载荷键

    Container.AddNode(CustomDemoObjectNode); // 将节点添加到容器

    return true; // 翻译成功
}

// 记录翻译事件日志
void UCustomFooTranslatorExtended::LogTranslationEvent(const FString& EventType, const FString& Message, const FString& FilePath) const
{
    // 构建日志消息
    FString LogMessage = FString::Printf(TEXT("[%s] %s"), *EventType, *Message);
    if (!FilePath.IsEmpty())
    {
        LogMessage += FString::Printf(TEXT(" (File: %s)"), *FilePath); // 添加文件路径信息
    }

    // 根据事件类型选择日志级别
    if (EventType == TEXT("Error"))
    {
        UE_LOG(LogInterchangeEngine, Error, TEXT("%s"), *LogMessage); // 错误级别日志
    }
    else if (EventType == TEXT("Warning"))
    {
        UE_LOG(LogInterchangeEngine, Warning, TEXT("%s"), *LogMessage); // 警告级别日志
    }
    else
    {
        UE_LOG(LogInterchangeEngine, Log, TEXT("%s"), *LogMessage); // 信息级别日志
    }
}

// 初始化默认翻译选项
void UCustomFooTranslatorExtended::InitializeDefaultOptions()
{
    TranslationOptions.Add(TEXT("EnableDetailedLogging"), TEXT("true")); // 启用详细日志
    TranslationOptions.Add(TEXT("ValidateInputFiles"), TEXT("true")); // 验证输入文件
    TranslationOptions.Add(TEXT("CreateBackups"), TEXT("false")); // 不创建备份
    TranslationOptions.Add(TEXT("OverwriteExisting"), TEXT("false")); // 不覆盖现有文件
}

// 更新翻译统计信息
void UCustomFooTranslatorExtended::UpdateStatistics(bool bSuccess, double TranslationTime) const
{
    TranslationCount++; // 增加翻译计数
    TotalTranslationTime += TranslationTime; // 累加翻译时间

    if (bSuccess)
    {
        SuccessfulTranslations++; // 增加成功计数
    }
    else
    {
        FailedTranslations++; // 增加失败计数
    }
}

// Python翻译器管理器实现

// 创建翻译器实例
UCustomFooTranslatorExtended* UPythonTranslatorManager::CreateTranslator()
{
    return NewObject<UCustomFooTranslatorExtended>(); // 创建新的扩展翻译器实例
}

// 获取可用的翻译器类型列表
TArray<FString> UPythonTranslatorManager::GetAvailableTranslatorTypes()
{
    TArray<FString> TranslatorTypes;
    TranslatorTypes.Add(TEXT("CustomFooTranslatorExtended")); // 添加扩展翻译器类型
    return TranslatorTypes;
}

// 为指定文件查找最佳翻译器
FString UPythonTranslatorManager::FindBestTranslatorForFile(const FString& FilePath)
{
    UCustomFooTranslatorExtended* Translator = CreateTranslator(); // 创建翻译器实例
    if (Translator && Translator->CanTranslateFile(FilePath)) // 检查是否可以翻译该文件
    {
        return TEXT("CustomFooTranslatorExtended"); // 返回翻译器类型名
    }

    return TEXT(""); // 没有找到合适的翻译器
}

// 批量翻译文件
int32 UPythonTranslatorManager::BatchTranslateFiles(const TArray<FString>& FilePaths,
                                                   const FString& DestinationPath,
                                                   TArray<FString>& OutResults)
{
    int32 SuccessCount = 0; // 成功计数器
    OutResults.Empty(); // 清空结果数组

    // 创建翻译器实例
    UCustomFooTranslatorExtended* Translator = CreateTranslator();
    if (!Translator)
    {
        return 0; // 创建翻译器失败
    }

    // 遍历所有文件路径
    for (const FString& FilePath : FilePaths)
    {
        bool bSuccess = Translator->TranslateFileFromPython(FilePath, DestinationPath); // 翻译单个文件
        OutResults.Add(bSuccess ? TEXT("Success") : TEXT("Failed")); // 添加结果状态

        if (bSuccess)
        {
            SuccessCount++; // 增加成功计数
        }
    }

    return SuccessCount; // 返回成功翻译的文件数量
}
