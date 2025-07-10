// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Nodes/InterchangeBaseNode.h"
#include "UObject/Class.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "UObject/UObjectGlobals.h"

#include "CustomDemoObjectNode.generated.h"

class UInterchangeBaseNodeContainer;

//Interchange namespace
namespace UE
{
	namespace Interchange
	{
		struct CUSTOMNODES_API FCustomDemoObjectNodeStaticData : public FBaseNodeStaticData
		{
			static const FAttributeKey& PayLoadKey();
		};

	}//ns Interchange
}//ns UE

UCLASS(BlueprintType)
class CUSTOMNODES_API UCustomDemoObjectNode : public UInterchangeBaseNode
{
	GENERATED_BODY()

	UCustomDemoObjectNode();

public:
	/**
	 * Initialize the base data of the node
	 * @param UniqueID - The uniqueId for this node
	 * @param DisplayLabel - The name of the node
	 *
	 */
	UFUNCTION(BlueprintCallable, Category = "Interchange | Node | DemoObject")
	void InitializeCustomDemoObjectNode(const FString& UniqueID, const FString& DisplayLabel);

	/**
	 * Return the node type name of the class, we use this when reporting error
	 */
	virtual FString GetTypeName() const override;

	virtual const TOptional<FString> GetPayLoadKey() const;

	UFUNCTION(BlueprintCallable, Category = "Interchange | Node | DemoObject")
	virtual void SetPayLoadKey(const FString& PayLoadKey);
};
