// Copyright Epic Games, Inc. All Rights Reserved.

/**
 * This class should be defined in your game, we define it here because we need a UObject to document a custom asset import
 */

#pragma once

#include "CoreMinimal.h"
#include "EditorFramework/AssetImportData.h"
#include "Engine/EngineTypes.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

#include "DemoObject.generated.h"

UENUM()
enum class EDemoObjectUnit : uint8
{
	/** millimeter */
	Millimeter,
	/** centimeter */
	Centimeter,
	/** meter */
	Meter
};

UCLASS(hidecategories = Object, MinimalAPI)
class UDemoObject : public UObject
{
	GENERATED_BODY()
public:
	/** A boolean setting to configure this asset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DemoObject")
	bool bAssetSettingBoolean = false;

	/** A float setting to configure this asset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DemoObject")
	float AssetSettingFloat = 0.0f;

	/** A string setting to configure this asset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DemoObject")
	FString AssetSettingString = TEXT("Not Set");

	/** A enum setting to configure this asset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DemoObject")
	EDemoObjectUnit AssetSettingEnum = EDemoObjectUnit::Centimeter;

	/** This is the data we need to build the asset. This data can be imported from a .foo file or generated. */
	UPROPERTY(VisibleAnywhere, Category = "SourceData")
	FString SourceData;

	/** If the SourceData was imported, we store the reimport information here (last import filename). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReimportData")
	TObjectPtr<UAssetImportData> AssetImportData = nullptr;

};

