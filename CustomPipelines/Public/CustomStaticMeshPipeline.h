// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InterchangePipelineBase.h"
#include "CustomStaticMeshPipeline.generated.h"

class UInterchangeBaseNodeContainer;
class UInterchangeSourceData;
class UStaticMeshDescription;

/**
 * 
 */
UCLASS()
class CUSTOMPIPELINES_API UCustomStaticMeshPipeline : public UInterchangePipelineBase
{
	GENERATED_BODY()
	
public:
	/** The name of the pipeline that will be display in the import dialog. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CustomStaticMesh", meta = (StandAlonePipelineProperty = "True", PipelineInternalEditionData = "True"))
	FString PipelineDisplayName;

	/** If enabled, imports the DemoObject assets found in the sources. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CustomStaticMesh")
	bool bImportCustomStaticMeshWedges = true;

	/** If enabled, imports the DemoObject assets found in the sources. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CustomStaticMesh")
	float Thickness = 1.0f;

protected:
	virtual void ExecutePipeline(UInterchangeBaseNodeContainer* BaseNodeContainer, const TArray<UInterchangeSourceData*>& SourceDatas, const FString& ContentBasePath) override;
	virtual void ExecutePostFactoryPipeline(const UInterchangeBaseNodeContainer* BaseNodeContainer, const FString& NodeKey, UObject* CreatedAsset, bool bIsAReimport) override;
private:
	const FName WedgeMaterialName = "M_Wedge";
	FString GetWedgeCountKey();
	FString BuildWedgeKey(int32 WedgeIndex, bool bIsV0);
	void BuildWedgeMeshDescription(const FVector3f& PositionA, const FVector3f& PositionB, UStaticMeshDescription* OutWedgeDescription, FVector3f& OutCenter);
};
