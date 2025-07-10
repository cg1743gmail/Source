// Copyright Epic Games, Inc. All Rights Reserved.

#include "CustomDemoObjectFactory.h"

#include "Async/ParallelFor.h"
#include "Async/TaskGraphInterfaces.h"
#include "DemoObject.h"
#include "Engine/Texture.h"
#include "Engine/Texture2D.h"
#include "InterchangeAssetImportData.h"
#include "InterchangeImportCommon.h"
#include "CustomImportLog.h"
#include "InterchangeResult.h"
#include "CustomDemoObjectFactoryNode.h"
#include "CustomDemoObjectNode.h"
#include "InterchangeTranslatorBase.h"
#include "Nodes/InterchangeBaseNode.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "CustomDemoObjectPayloadInterface.h"
#include "CustomFooTranslator.h"
#include "TextureCompiler.h"
#include "TextureImportSettings.h"
#include "TextureResource.h"
#include "UObject/ObjectMacros.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CustomDemoObjectFactory)

#if WITH_EDITORONLY_DATA

#include "EditorFramework/AssetImportData.h"

#endif //WITH_EDITORONLY_DATA

#define LOCTEXT_NAMESPACE "CustomDemoObjectFactory"


UClass* UCustomDemoObjectFactory::GetFactoryClass() const
{
	return UDemoObject::StaticClass();
}

UInterchangeFactoryBase::FImportAssetResult UCustomDemoObjectFactory::BeginImportAsset_GameThread(const FImportAssetObjectParams& Arguments)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UCustomDemoObjectFactory::BeginImportAsset_GameThread);
	FImportAssetResult ImportAssetResult;
	UDemoObject* DemoObjectAsset = nullptr;

	auto CouldNotCreateDemoObjectLog = [this, &Arguments, &ImportAssetResult](const FText& Info)
	{
		UInterchangeResultError_Generic* Message = AddMessage<UInterchangeResultError_Generic>();
		Message->SourceAssetName = Arguments.SourceData->GetFilename();
		Message->DestinationAssetName = Arguments.AssetName;
		Message->AssetType = GetFactoryClass();
		Message->Text = FText::Format(LOCTEXT("TexFact_CouldNotCreateMat", "UCustomDemoObjectFactory: Could not create DemoObject asset %s. Reason: %s"), FText::FromString(Arguments.AssetName), Info);
		bSkipImport = true;
		ImportAssetResult.bIsFactorySkipAsset = true;
	};

	UCustomDemoObjectFactoryNode* DemoObjectFactoryNode = Cast<UCustomDemoObjectFactoryNode>(Arguments.AssetNode);
	if (!DemoObjectFactoryNode)
	{
		CouldNotCreateDemoObjectLog(LOCTEXT("DemoObjectFactory_AssetNodeNull", "Asset node parameter is null."));
		return ImportAssetResult;
	}

	const UClass* DemoObjectClass = Arguments.AssetNode->GetObjectClass();
	if (!DemoObjectClass || !DemoObjectClass->IsChildOf(UDemoObject::StaticClass()))
	{
		CouldNotCreateDemoObjectLog(LOCTEXT("DemoObjectFactory_NodeClassMissmatch", "Asset node parameter class doesnt derive from UDemoObject."));
		return ImportAssetResult;
	}

	const bool bIsReimport = Arguments.ReimportObject != nullptr;

	UObject* ExistingAsset = Arguments.ReimportObject;
	if (!ExistingAsset)
	{
		FSoftObjectPath ReferenceObject;
		if (Arguments.AssetNode->GetCustomReferenceObject(ReferenceObject))
		{
			ExistingAsset = ReferenceObject.TryLoad();
		}
	}

	// create a new DemoObjectAsset or overwrite existing asset, if possible
	if (!ExistingAsset)
	{
		DemoObjectAsset = NewObject<UDemoObject>(Arguments.Parent, DemoObjectClass, *Arguments.AssetName, RF_Public | RF_Standalone);
	}
	else
	{
		DemoObjectAsset = Cast<UDemoObject>(ExistingAsset);
		//We allow override of existing DemoObjects only if the translator is a pure DemoObject translator or the user directly ask to re-import this object
		if (!bIsReimport && Arguments.Translator->GetSupportedAssetTypes() != EInterchangeTranslatorAssetType::Textures)
		{
			//Do not override the material asset
			ImportAssetResult.bIsFactorySkipAsset = true;
			bSkipImport = true;
		}
	}

	if (!DemoObjectAsset)
	{
		CouldNotCreateDemoObjectLog(LOCTEXT("DemoObjectFactory_DemoObjectCreateFail", "DemoObject creation fail."));
		return ImportAssetResult;
	}

	ImportAssetResult.ImportedObject = DemoObjectAsset;
	return ImportAssetResult;
}

// The payload fetching and the heavy operations are done here
UInterchangeFactoryBase::FImportAssetResult UCustomDemoObjectFactory::ImportAsset_Async(const FImportAssetObjectParams& Arguments)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UCustomDemoObjectFactory::ImportAsset_Async);

	FImportAssetResult ImportAssetResult;
	ImportAssetResult.bIsFactorySkipAsset = bSkipImport;

	auto ImportDemoObjectErrorLog = [this, &Arguments](const FText& Info)
	{
		UInterchangeResultError_Generic* Message = AddMessage<UInterchangeResultError_Generic>();
		Message->SourceAssetName = Arguments.SourceData->GetFilename();
		Message->DestinationAssetName = Arguments.AssetName;
		Message->AssetType = GetFactoryClass();
		Message->Text = Info;
	};

	if (!Arguments.AssetNode)
	{
		ImportDemoObjectErrorLog(LOCTEXT("DemoObjectFactory_Async_AssetNodeNull", "UCustomDemoObjectFactory: Asset node parameter is null."));
		return ImportAssetResult;
	}

	const UClass* DemoObjectClass = Arguments.AssetNode->GetObjectClass();
	if (!DemoObjectClass || !DemoObjectClass->IsChildOf(UDemoObject::StaticClass()))
	{
		ImportDemoObjectErrorLog(LOCTEXT("DemoObjectFactory_Async_MissMatchClass", "UCustomDemoObjectFactory: Asset node parameter class doesnt derive from UDemoObject."));
		return ImportAssetResult;
	}

	UObject* ExistingAsset = nullptr;
	FSoftObjectPath ReferenceObject;
	if (Arguments.AssetNode->GetCustomReferenceObject(ReferenceObject))
	{
		ExistingAsset = ReferenceObject.TryLoad();
	}

	//Do not override an asset we skip
	if (bSkipImport)
	{
		ImportAssetResult.ImportedObject = ExistingAsset;
		return ImportAssetResult;
	}

	UCustomDemoObjectFactoryNode* DemoObjectFactoryNode = Cast<UCustomDemoObjectFactoryNode>(Arguments.AssetNode);
	if (!DemoObjectFactoryNode)
	{
		ImportDemoObjectErrorLog(LOCTEXT("DemoObjectFactory_Async_WrongObjectType", "UCustomDemoObjectFactory: Asset node type is not UCustomDemoObjectFactoryNode."));
		return ImportAssetResult;
	}

	const TOptional<FString>& PayLoadKey = DemoObjectFactoryNode->GetPayLoadKey();
	if (!PayLoadKey.IsSet())
	{
		ImportDemoObjectErrorLog(LOCTEXT("DemoObjectFactory_Async_InvalidPayloadKey", "UCustomDemoObjectFactory: DemoObject translated node (UInterchangeDemoObjectNode) doesnt have a payload key."));
		return ImportAssetResult;
	}

	const ICustomDemoObjectPayloadInterface* CustomDemoObjectPayloadInterface = Cast<ICustomDemoObjectPayloadInterface>(Arguments.Translator);
	if (!CustomDemoObjectPayloadInterface)
	{
		UE_LOG(LogCustomImport, Error, TEXT("Cannot import DemoObject, the translator does not implement the ICustomDemoObjectPayloadInterface."));
		return ImportAssetResult;
	}

	TOptional<UE::Interchange::FCustomDemoObjectData> PayloadData = CustomDemoObjectPayloadInterface->GetDemoObjectPayloadData(PayLoadKey.GetValue());

	if(!PayloadData.IsSet())
	{
		ImportDemoObjectErrorLog(LOCTEXT("DemoObjectFactory_Async_CannotRetrievePayload", "UCustomDemoObjectFactory: Invalid translator couldn't retrive a payload."));
		return ImportAssetResult;
	}

	UDemoObject* DemoObjectAsset = nullptr;
	// create a new DemoObject or overwrite existing asset, if possible
	if (!ExistingAsset)
	{
		ImportDemoObjectErrorLog(LOCTEXT("DemoObjectFactory_Async_CannotCreateAsync", "UCustomDemoObjectFactory: Could not create DemoObject asset outside of the game thread."));
		return ImportAssetResult;
	}
	else
	{
		DemoObjectAsset = Cast<UDemoObject>(ExistingAsset);
	}

	if (!DemoObjectAsset)
	{
		ImportDemoObjectErrorLog(LOCTEXT("DemoObjectFactory_Async_FailCreatingAsset", "UCustomDemoObjectFactory: Could not create DemoObject asset."));
		return ImportAssetResult;
	}

	//TODO....
	//Use the payload to fill the source data
	DemoObjectAsset->SourceData = PayloadData->FormatJsonData;

	ImportAssetResult.ImportedObject = DemoObjectAsset;
	return ImportAssetResult;
}

/* This function is call in the completion task on the main thread, use it to call main thread post creation step for your assets*/
void UCustomDemoObjectFactory::SetupObject_GameThread(const FSetupObjectParams& Arguments)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UCustomDemoObjectFactory::SetupObject_GameThread);

	check(IsInGameThread());

	if (bSkipImport)
	{
		return;
	}

	UDemoObject* DemoObjectAsset = Cast<UDemoObject>(Arguments.ImportedObject);

	// Finish the import on the game thread by doing the setup on the DemoObject asset here
	if (!DemoObjectAsset)
	{
		return;
	}

#if WITH_EDITOR
	DemoObjectAsset->PreEditChange(nullptr);
#endif

#if WITH_EDITOR
	UInterchangeFactoryBaseNode* DemoObjectFactoryNode = Arguments.FactoryNode;
	if (!Arguments.bIsReimport)
	{
		/** Apply all DemoObjectNode custom attributes to the DemoObject asset */
		DemoObjectFactoryNode->ApplyAllCustomAttributeToObject(DemoObjectAsset);
	}
	else
	{
		UInterchangeAssetImportData* InterchangeAssetImportData = Cast<UInterchangeAssetImportData>(DemoObjectAsset->AssetImportData);
		UInterchangeFactoryBaseNode* PreviousNode = nullptr;
		if (InterchangeAssetImportData)
		{
			PreviousNode = InterchangeAssetImportData->GetStoredFactoryNode(InterchangeAssetImportData->NodeUniqueID);
		}

		UInterchangeFactoryBaseNode* CurrentNode = NewObject<UInterchangeFactoryBaseNode>(GetTransientPackage(), UCustomDemoObjectFactoryNode::StaticClass());
		UInterchangeBaseNode::CopyStorage(DemoObjectFactoryNode, CurrentNode);
		CurrentNode->FillAllCustomAttributeFromObject(DemoObjectAsset);
		//Apply reimport strategy
		UE::Interchange::FFactoryCommon::ApplyReimportStrategyToAsset(DemoObjectAsset, PreviousNode, CurrentNode, DemoObjectFactoryNode);
	}
#endif // WITH_EDITOR

	Super::SetupObject_GameThread(Arguments);

	//TODO make sure this work at runtime
#if WITH_EDITORONLY_DATA
	if (ensure(DemoObjectAsset && Arguments.SourceData))
	{
		//We must call the Update of the asset source file in the main thread because UAssetImportData::Update execute some delegate we do not control
		UE::Interchange::FFactoryCommon::FSetImportAssetDataParameters SetImportAssetDataParameters(DemoObjectAsset
																						 , DemoObjectAsset->AssetImportData
																						 , Arguments.SourceData
																						 , Arguments.NodeUniqueID
																						 , Arguments.NodeContainer
																						 , Arguments.OriginalPipelines
																						 , Arguments.Translator);
		SetImportAssetDataParameters.SourceFiles = MoveTemp(SourceFiles);

		DemoObjectAsset->AssetImportData = UE::Interchange::FFactoryCommon::SetImportAssetData(SetImportAssetDataParameters);
	}
#endif
}


bool UCustomDemoObjectFactory::GetSourceFilenames(const UObject* Object, TArray<FString>& OutSourceFilenames) const
{
#if WITH_EDITORONLY_DATA
	if (const UDemoObject* DemoObjectAsset = Cast<UDemoObject>(Object))
	{
		return UE::Interchange::FFactoryCommon::GetSourceFilenames(DemoObjectAsset->AssetImportData.Get(), OutSourceFilenames);
	}
#endif

	return false;
}

bool UCustomDemoObjectFactory::SetSourceFilename(const UObject* Object, const FString& SourceFilename, int32 SourceIndex) const
{
#if WITH_EDITORONLY_DATA
	if (const UDemoObject* DemoObjectAsset = Cast<UDemoObject>(Object))
	{
		return UE::Interchange::FFactoryCommon::SetSourceFilename(DemoObjectAsset->AssetImportData.Get(), SourceFilename, SourceIndex);
	}
#endif

	return false;
}

#undef LOCTEXT_NAMESPACE
