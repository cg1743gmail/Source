// Copyright Epic Games, Inc. All Rights Reserved. 
#include "CustomFooTranslator.h"

#include "CustomImportLog.h"
#include "CustomDemoObjectNode.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "Nodes/InterchangeBaseNodeContainer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CustomFooTranslator)

static bool GInterchangeEnableCustomFooImport = true;
static FAutoConsoleVariableRef CCvarInterchangeEnableCustomFooImport(
	TEXT("Interchange.FeatureFlags.Import.Foo"),
	GInterchangeEnableCustomFooImport,
	TEXT("Whether Foo support is enabled."),
	ECVF_Default);

TArray<FString> UCustomFooTranslator::GetSupportedFormats() const
{
	TArray<FString> Formats;

	if (GInterchangeEnableCustomFooImport || GIsAutomationTesting)
	{
		Formats.Add(TEXT("foo;FOO my game format"));
	}

	return Formats;
}

bool UCustomFooTranslator::Translate(UInterchangeBaseNodeContainer& BaseNodeContainer) const
{
	FString Filename = SourceData->GetFilename();
	FPaths::NormalizeFilename(Filename);
	if (!FPaths::FileExists(Filename))
	{
		return false;
	}

	UClass* Class = UCustomDemoObjectNode::StaticClass();
	if (!ensure(Class))
	{
		return false;
	}

	FString DisplayLabel = FPaths::GetBaseFilename(Filename);
	FString NodeUID(Filename);
	UCustomDemoObjectNode* CustomDemoObjectNode = NewObject<UCustomDemoObjectNode>(&BaseNodeContainer, Class);
	if (!ensure(CustomDemoObjectNode))
	{
		return false;
	}

	CustomDemoObjectNode->InitializeCustomDemoObjectNode(NodeUID, DisplayLabel);
	CustomDemoObjectNode->SetPayLoadKey(Filename);

	BaseNodeContainer.AddNode(CustomDemoObjectNode);

	return true;
}

TOptional<UE::Interchange::FCustomDemoObjectData> UCustomFooTranslator::GetDemoObjectPayloadData(const FString& PayloadKey) const
{
	UE::Interchange::FCustomDemoObjectData CustomDemoObjectData;
	//Open the MyGameFormat file and extract the data to return it
	FFileHelper::LoadFileToString(CustomDemoObjectData.FormatJsonData, *SourceData->GetFilename());
	return MoveTemp(CustomDemoObjectData);
}

