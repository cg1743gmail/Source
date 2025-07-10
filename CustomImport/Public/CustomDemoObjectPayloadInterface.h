// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "InterchangeSourceData.h"
#include "CustomDemoObjectPayloadData.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

#include "CustomDemoObjectPayloadInterface.generated.h"

UINTERFACE()
class CUSTOMIMPORT_API UCustomDemoObjectPayloadInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * CustomDemoObject payload interface. Derive from it if your translator can import DemoObject
 */
class CUSTOMIMPORT_API ICustomDemoObjectPayloadInterface
{
	GENERATED_BODY()
public:

	/**
	 * Once the translation is done, the import process need a way to retrieve payload data.
	 * This payload will be use by the factories to create the asset.
	 *
	 * @param PayloadKey - The key to retrieve the a particular payload contain into the specified source data.
	 * @return a PayloadData containing the import image data. The TOptional will not be set if there is an error.
	 */
	virtual TOptional<UE::Interchange::FCustomDemoObjectData> GetDemoObjectPayloadData(const FString& PayloadKey) const PURE_VIRTUAL(ICustomDemoObjectPayloadInterface::GetMyGameFormatPayloadData, return{};);
};


