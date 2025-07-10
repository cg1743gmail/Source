// Copyright Epic Games, Inc. All Rights Reserved. 

#include "CustomDemoObjectPipeline.h"

#include "CustomPipelineLog.h"
#include "CustomDemoObjectFactoryNode.h"
#include "CustomDemoObjectNode.h"
#include "Misc/Paths.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CustomDemoObjectPipeline)



#if WITH_EDITOR

void UCustomDemoObjectPipeline::FilterPropertiesFromTranslatedData(UInterchangeBaseNodeContainer* InBaseNodeContainer)
{
	Super::FilterPropertiesFromTranslatedData(InBaseNodeContainer);

	//Filter all material pipeline properties if there is no translated material.
	TArray<FString> TmpCustomDemoObjectNodes;
	InBaseNodeContainer->GetNodes(UCustomDemoObjectNode::StaticClass(), TmpCustomDemoObjectNodes);
	if (TmpCustomDemoObjectNodes.Num() == 0)
	{
		//Filter out all DemoObjects properties
		if (UInterchangePipelineBase* OuterMostPipeline = GetMostPipelineOuter())
		{
			HidePropertiesOfCategory(OuterMostPipeline, this, TEXT("DemoObjects"));
		}
	}
}

#endif //WITH_EDITOR

void UCustomDemoObjectPipeline::ExecutePipeline(UInterchangeBaseNodeContainer* InBaseNodeContainer, const TArray<UInterchangeSourceData*>& InSourceDatas, const FString& ContentBasePath)
{
	if (!InBaseNodeContainer)
	{
		UE_LOG(LogCustomPipeline, Warning, TEXT("UCustomDemoObjectPipeline: Cannot execute pre-import pipeline because InBaseNodeContrainer is null"));
		return;
	}

	BaseNodeContainer = InBaseNodeContainer;
	SourceDatas.Empty(InSourceDatas.Num());
	for (const UInterchangeSourceData* SourceData : InSourceDatas)
	{
		SourceDatas.Add(SourceData);
	}
	
	//Find all translated node we need for this pipeline
	BaseNodeContainer->IterateNodes([this](const FString& NodeUid, UInterchangeBaseNode* Node)
	{
		switch(Node->GetNodeContainerType())
		{
			case EInterchangeNodeContainerType::TranslatedAsset:
			{
				if (UCustomDemoObjectNode* CustomDemoObjectNode = Cast<UCustomDemoObjectNode>(Node))
				{
					CustomDemoObjectNodes.Add(CustomDemoObjectNode);
				}
			}
			break;
		}
	});

	if (bImportDemoObjects)
	{
		UCustomDemoObjectFactoryNode* DemoObjectFactoryNode = nullptr;
		for (UCustomDemoObjectNode* CustomDemoObjectNode : CustomDemoObjectNodes)
		{
			DemoObjectFactoryNode = CreateDemoObjectFactoryNode(CustomDemoObjectNode);
		}
		//If we have a valid override name
		FString OverrideAssetName = IsStandAlonePipeline() ? DestinationName : FString();
		if (OverrideAssetName.IsEmpty() && IsStandAlonePipeline())
		{
			OverrideAssetName = AssetName;
		}

		const bool bOverrideAssetName = CustomDemoObjectNodes.Num() == 1 && IsStandAlonePipeline() && !OverrideAssetName.IsEmpty();
		if (DemoObjectFactoryNode && bOverrideAssetName)
		{
			DemoObjectFactoryNode->SetAssetName(OverrideAssetName);
			DemoObjectFactoryNode->SetDisplayLabel(OverrideAssetName);
		}
	}
}

UCustomDemoObjectFactoryNode* UCustomDemoObjectPipeline::CreateDemoObjectFactoryNode(const UCustomDemoObjectNode* CustomDemoObjectNode)
{
	FString DisplayLabel = CustomDemoObjectNode->GetDisplayLabel();
	FString NodeUid = UInterchangeFactoryBaseNode::BuildFactoryNodeUid(CustomDemoObjectNode->GetUniqueID());
	UCustomDemoObjectFactoryNode* DemoObjectFactoryNode = nullptr;
	if (BaseNodeContainer->IsNodeUidValid(NodeUid))
	{
		DemoObjectFactoryNode = Cast<UCustomDemoObjectFactoryNode>(BaseNodeContainer->GetFactoryNode(NodeUid));
		if (!ensure(DemoObjectFactoryNode))
		{
			//Log an error
			return nullptr;
		}
	}
	else
	{
		DemoObjectFactoryNode = NewObject<UCustomDemoObjectFactoryNode>(BaseNodeContainer);
		if (!ensure(DemoObjectFactoryNode))
		{
			return nullptr;
		}
		//Creating a DemoObject factory node
		DemoObjectFactoryNode->InitializeNode(NodeUid, DisplayLabel, EInterchangeNodeContainerType::FactoryData);

		TOptional<FString> PayloadKey = CustomDemoObjectNode->GetPayLoadKey();
		if (!PayloadKey.IsSet())
		{
			return nullptr;
		}
		DemoObjectFactoryNode->SetPayLoadKey(PayloadKey.GetValue());
		constexpr bool bAddApplyDelegate = true; //We want to add the delegate that allow the factory to set all the settings
		DemoObjectFactoryNode->SetCustombAssetSettingBoolean(bAssetSettingBoolean, bAddApplyDelegate);
		DemoObjectFactoryNode->SetCustomAssetSettingFloat(AssetSettingFloat, bAddApplyDelegate);
		DemoObjectFactoryNode->SetCustomAssetSettingString(AssetSettingString, bAddApplyDelegate);
		DemoObjectFactoryNode->SetCustomAssetSettingEnum(AssetSettingEnum, bAddApplyDelegate);

		BaseNodeContainer->AddNode(DemoObjectFactoryNode);
		CustomDemoObjectFactoryNodes.Add(DemoObjectFactoryNode);

		DemoObjectFactoryNode->AddTargetNodeUid(CustomDemoObjectNode->GetUniqueID());
		CustomDemoObjectNode->AddTargetNodeUid(DemoObjectFactoryNode->GetUniqueID());

	}
	return DemoObjectFactoryNode;
}


