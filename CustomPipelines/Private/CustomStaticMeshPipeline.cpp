// Copyright Epic Games, Inc. All Rights Reserved.


#include "CustomStaticMeshPipeline.h"

#include "Engine/StaticMesh.h"
#include "InterchangeSceneNode.h"
#include "InterchangeSourceData.h"
#include "InterchangeStaticMeshFactoryNode.h"
#include "MaterialDomain.h"
#include "Materials/Material.h"
#include "Misc/CString.h"
#include "Nodes/InterchangeBaseNodeContainer.h"
#include "MeshDescription.h"
#include "StaticMeshAttributes.h"
#include "StaticMeshOperations.h"
#include "StaticMeshDescription.h"

void UCustomStaticMeshPipeline::ExecutePipeline(UInterchangeBaseNodeContainer* BaseNodeContainer, const TArray<UInterchangeSourceData*>& SourceDatas, const FString& ContentBasePath)
{
	if (!BaseNodeContainer || !bImportCustomStaticMeshWedges)
	{
		return;
	}
	TArray<FString> StaticMeshFactoryNodes;
	BaseNodeContainer->GetNodes(UInterchangeStaticMeshFactoryNode::StaticClass(), StaticMeshFactoryNodes);
	if (StaticMeshFactoryNodes.Num() == 1)
	{
		UInterchangeStaticMeshFactoryNode* StaticMeshFactoryNode = Cast<UInterchangeStaticMeshFactoryNode>(BaseNodeContainer->GetFactoryNode(StaticMeshFactoryNodes[0]));
		//Find the wedges
		TMap<int32, TPair<const UInterchangeSceneNode*, const UInterchangeSceneNode*>> WedgeNodes;
		BaseNodeContainer->IterateNodesOfType<UInterchangeSceneNode>([&WedgeNodes](const FString& NodeUid, const UInterchangeSceneNode* SceneNode)
			{
				FString NodeName = SceneNode->GetDisplayLabel();
				if (NodeName.Len() == 10 && NodeName.StartsWith(TEXT("Edge"), ESearchCase::IgnoreCase))
				{
					if(NodeName.Find(TEXT("_V"), ESearchCase::IgnoreCase, ESearchDir::FromStart) == 7)
					{
						int32 EdgeIndex = FCString::Atoi(*NodeName.Mid(4, 3));
						bool bStartVector = NodeName.EndsWith(TEXT("0"));
						bool bInitWedge = !WedgeNodes.Contains(EdgeIndex);
						TPair<const UInterchangeSceneNode*, const UInterchangeSceneNode*>& Wedge = WedgeNodes.FindOrAdd(EdgeIndex);
						if (bInitWedge)
						{
							Wedge.Key = nullptr;
							Wedge.Value = nullptr;
						}
						bStartVector ? Wedge.Key = SceneNode : Wedge.Value = SceneNode;
					}
				}
			});

		if (WedgeNodes.Num() > 0)
		{
			TArray<FTransform> TransformPairPerWedges;
			WedgeNodes.KeySort(TLess<int32>());
			int32 WedgeIndex = 0;
			for (TPair<int32, TPair<const UInterchangeSceneNode*, const UInterchangeSceneNode*>>& Wedge : WedgeNodes)
			{
				const UInterchangeSceneNode* InterchangeSceneNodeV0 = Wedge.Value.Key;
				const UInterchangeSceneNode* InterchangeSceneNodeV1 = Wedge.Value.Value;
				if (!InterchangeSceneNodeV0 || !InterchangeSceneNodeV1)
				{
					continue;
				}

				FTransform WedgeV0;
				if (!InterchangeSceneNodeV0->GetCustomGlobalTransform(BaseNodeContainer, FTransform::Identity, WedgeV0))
				{
					continue;
				}
				FTransform WedgeV1;
				if (!InterchangeSceneNodeV1->GetCustomGlobalTransform(BaseNodeContainer, FTransform::Identity, WedgeV1))
				{
					continue;
				}
				TransformPairPerWedges.Add(WedgeV0);
				TransformPairPerWedges.Add(WedgeV1);
				StaticMeshFactoryNode->RegisterAttribute<FTransform>(UE::Interchange::FAttributeKey(BuildWedgeKey(WedgeIndex, true)), WedgeV0);
				StaticMeshFactoryNode->RegisterAttribute<FTransform>(UE::Interchange::FAttributeKey(BuildWedgeKey(WedgeIndex, false)), WedgeV1);
				WedgeIndex++;
			}
			if (WedgeIndex > 0)
			{
				StaticMeshFactoryNode->AddInt32Attribute(GetWedgeCountKey(), WedgeIndex);
			}
		}
	}
}

void UCustomStaticMeshPipeline::ExecutePostFactoryPipeline(const UInterchangeBaseNodeContainer* BaseNodeContainer, const FString& NodeKey, UObject* CreatedAsset, bool bIsAReimport)
{
	UInterchangeStaticMeshFactoryNode* StaticMeshFactoryNode = Cast<UInterchangeStaticMeshFactoryNode>(BaseNodeContainer->GetFactoryNode(NodeKey));
	if (!StaticMeshFactoryNode)
	{
		return;
	}

	UStaticMesh* StaticMesh = Cast<UStaticMesh>(CreatedAsset);
	if (!StaticMesh)
	{
		return;
	}

	FMeshDescription* MeshDescription = StaticMesh->GetMeshDescription(0);
	if (!MeshDescription)
	{
		return;
	}

	FStaticMeshOperations::FAppendSettings AppendSettings;
	for (int32 ChannelIdx = 0; ChannelIdx < FStaticMeshOperations::FAppendSettings::MAX_NUM_UV_CHANNELS; ++ChannelIdx)
	{
		AppendSettings.bMergeUVChannels[ChannelIdx] = true;
	}

	int32 WedgeCount = 0;
	StaticMeshFactoryNode->GetInt32Attribute(GetWedgeCountKey(), WedgeCount);
	
	if (WedgeCount > 0)
	{
		StaticMesh->GetStaticMaterials().Emplace(UMaterial::GetDefaultMaterial(MD_Surface), WedgeMaterialName);
	}

	for (int32 WedgeIndex = 0; WedgeIndex < WedgeCount; ++WedgeIndex)
	{
		FTransform WedgeV0;
		FTransform WedgeV1;
		StaticMeshFactoryNode->GetAttribute<FTransform>(BuildWedgeKey(WedgeIndex, true), WedgeV0);
		StaticMeshFactoryNode->GetAttribute<FTransform>(BuildWedgeKey(WedgeIndex, false), WedgeV1);
		FVector3f PositionA = FVector3f(WedgeV0.GetTranslation());
		FVector3f PositionB = FVector3f(WedgeV1.GetTranslation());
		if (PositionA.Equals(PositionB))
		{
			continue;
		}
		UStaticMeshDescription* StaticMeshDescription = UStaticMesh::CreateStaticMeshDescription();
		FVector3f BoxOffset = FVector3f::ZeroVector;
		BuildWedgeMeshDescription(PositionA, PositionB, StaticMeshDescription, BoxOffset);
		AppendSettings.MeshTransform = FTransform(FVector(BoxOffset));
		FStaticMeshOperations::AppendMeshDescription(StaticMeshDescription->GetMeshDescription(), *MeshDescription, AppendSettings);
	}
	StaticMesh->CommitMeshDescription(0);
}

FString UCustomStaticMeshPipeline::GetWedgeCountKey()
{
	return TEXT("WedgeCount");
}

FString UCustomStaticMeshPipeline::BuildWedgeKey(int32 WedgeIndex, bool bIsV0)
{
	const FString BaseWedgeKey = TEXT("EdgeIndex_");
	const FString WedgeKeySeparatorV0 = TEXT("_V0");
	const FString WedgeKeySeparatorV1 = TEXT("_V1");
	FString Key = BaseWedgeKey + FString::FromInt(WedgeIndex) + (bIsV0 ? WedgeKeySeparatorV0 : WedgeKeySeparatorV1);
	return Key;
}

void UCustomStaticMeshPipeline::BuildWedgeMeshDescription(const FVector3f& PositionA, const FVector3f& PositionB, UStaticMeshDescription* OutWedgeDescription, FVector3f& OutCenter)
{
	FVector3f Delta = PositionB - PositionA;
	OutCenter = (Delta / 2) + PositionA;
	FVector3f HalfExtent = FVector3f(Thickness) + (Delta / 2);
	FPolygonGroupID PolygonGroupID = OutWedgeDescription->CreatePolygonGroup();
	OutWedgeDescription->SetPolygonGroupMaterialSlotName(PolygonGroupID, WedgeMaterialName);
	FPolygonID PolygonID_PlusX;
	FPolygonID PolygonID_MinusX;
	FPolygonID PolygonID_PlusY;
	FPolygonID PolygonID_MinusY;
	FPolygonID PolygonID_PlusZ;
	FPolygonID PolygonID_MinusZ;
	OutWedgeDescription->CreateCube(FVector::ZeroVector
		, FVector(HalfExtent)
		, PolygonGroupID
		, PolygonID_PlusX
		, PolygonID_MinusX
		, PolygonID_PlusY
		, PolygonID_MinusY
		, PolygonID_PlusZ
		, PolygonID_MinusZ);
}
