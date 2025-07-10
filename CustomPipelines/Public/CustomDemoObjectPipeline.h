// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "InterchangePipelineBase.h"
#include "InterchangeSourceData.h"
#include "CustomDemoObjectFactoryNode.h"
#include "Nodes/InterchangeBaseNodeContainer.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

#include "CustomDemoObjectPipeline.generated.h"

class UCustomDemoObjectNode;

UCLASS(BlueprintType, editinlinenew)
class CUSTOMPIPELINES_API UCustomDemoObjectPipeline : public UInterchangePipelineBase
{
	GENERATED_BODY()

public:
	/** The name of the pipeline that will be display in the import dialog. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DemoObjects", meta = (StandAlonePipelineProperty = "True", PipelineInternalEditionData = "True"))
	FString PipelineDisplayName;

	/** If enabled, imports the DemoObjects assets found in the sources. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DemoObjects")
	bool bImportDemoObjects = true;

	/** If not empty, and there is only one asset and one source data, we will name the asset with this string. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DemoObjects", meta=(StandAlonePipelineProperty = "True", AlwaysResetToDefault = "True"))
	FString AssetName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DemoObjects", Meta = (EditCondition = "bImportDemoObjects"))
	bool bAssetSettingBoolean = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DemoObjects", Meta = (EditCondition = "bImportDemoObjects"))
	float AssetSettingFloat = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DemoObjects", Meta = (EditCondition = "bImportDemoObjects"))
	FString AssetSettingString = TEXT("Not Set");
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DemoObjects", Meta = (EditCondition = "bImportDemoObjects"))
	EDemoObjectUnit AssetSettingEnum = EDemoObjectUnit::Centimeter;

public:
	virtual void ExecutePipeline(UInterchangeBaseNodeContainer* InBaseNodeContainer, const TArray<UInterchangeSourceData*>& InSourceDatas, const FString& ContentBasePath) override;

#if WITH_EDITOR

	virtual void FilterPropertiesFromTranslatedData(UInterchangeBaseNodeContainer* InBaseNodeContainer) override;

#endif //WITH_EDITOR

protected:
	UCustomDemoObjectFactoryNode* CreateDemoObjectFactoryNode(const UCustomDemoObjectNode* DemoObjectNode);

private:
	UPROPERTY()
	TObjectPtr<UInterchangeBaseNodeContainer> BaseNodeContainer;

	TArray<const UInterchangeSourceData*> SourceDatas;
	
	/** DemoObject translated assets nodes */
	TArray<UCustomDemoObjectNode*> CustomDemoObjectNodes;

	/** Texture factory assets nodes */
	TArray<UCustomDemoObjectFactoryNode*> CustomDemoObjectFactoryNodes;
};
