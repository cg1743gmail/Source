// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "InterchangeTranslatorBase.h"
#include "Nodes/InterchangeBaseNodeContainer.h"
#include "CustomDemoObjectPayloadData.h"
#include "CustomDemoObjectPayloadInterface.h"

#include "CustomFooTranslator.generated.h"

UCLASS(BlueprintType)
class CUSTOMIMPORT_API UCustomFooTranslator : public UInterchangeTranslatorBase, public ICustomDemoObjectPayloadInterface
{
	GENERATED_BODY()
public:

	virtual TArray<FString> GetSupportedFormats() const override;
	//Set this translator has a mesh
	virtual EInterchangeTranslatorAssetType GetSupportedAssetTypes() const override { return EInterchangeTranslatorAssetType::None; }

	/**
	 * Translate the associated source data into a node hold by the specified nodes container.
	 *
	 * @param BaseNodeContainer - The unreal objects descriptions container where to put the translated source data.
	 * @return true if the translator can translate the source data, false otherwise.
	 */
	virtual bool Translate(UInterchangeBaseNodeContainer& BaseNodeContainer) const override;



	/* ICustomFormatPayloadInterface Begin */

	/**
	 * Once the translation is done, the import process need a way to retrieve payload data.
	 * This payload will be use by the factories to create the asset.
	 *
	 * @param PayloadKey - The translator uses its SourceData property to extract the payload.
	 * @return a PayloadData containing the DemoObject data. The TOptional will not be set if there is an error.
	 */
	virtual TOptional<UE::Interchange::FCustomDemoObjectData> GetDemoObjectPayloadData(const FString& PayloadKey) const override;

	
	/* ICustomFormatPayloadInterface End */

};


