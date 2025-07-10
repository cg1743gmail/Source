// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorFramework/AssetImportData.h"
#include "InterchangeFactoryBase.h"
#include "CustomDemoObjectPayloadData.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

#include "CustomDemoObjectFactory.generated.h"

class UCustomDemoObjectFactoryNode;

UCLASS(BlueprintType, Experimental)
class CUSTOMIMPORT_API UCustomDemoObjectFactory : public UInterchangeFactoryBase
{
	GENERATED_BODY()
public:

	//////////////////////////////////////////////////////////////////////////
	// Interchange factory base interface begin

	virtual UClass* GetFactoryClass() const override;
	virtual EInterchangeFactoryAssetType GetFactoryAssetType() override { return EInterchangeFactoryAssetType::Textures; }
	virtual FImportAssetResult BeginImportAsset_GameThread(const FImportAssetObjectParams& Arguments) override;
	virtual FImportAssetResult ImportAsset_Async(const FImportAssetObjectParams& Arguments) override;
	virtual void SetupObject_GameThread(const FSetupObjectParams& Arguments) override;
	virtual bool GetSourceFilenames(const UObject* Object, TArray<FString>& OutSourceFilenames) const override;
	virtual bool SetSourceFilename(const UObject* Object, const FString& SourceFilename, int32 SourceIndex) const override;

	// Interchange factory base interface end
	//////////////////////////////////////////////////////////////////////////

private:

#if WITH_EDITORONLY_DATA
	//  The data for the source files will be stored here during the import
	TArray<FAssetImportInfo::FSourceFile> SourceFiles;
#endif // WITH_EDITORONLY_DATA

	//If we import without a pure texture translator, we should not override an existing texture and we must skip the import. See the implementation of BeginImportAssetObject_GameThread function.
	bool bSkipImport = false;
};


