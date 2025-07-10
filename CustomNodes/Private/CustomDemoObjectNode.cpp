// Copyright Epic Games, Inc. All Rights Reserved.

#include "CustomDemoObjectNode.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CustomDemoObjectNode)

//Interchange namespace
namespace UE
{
	namespace Interchange
	{
		const FAttributeKey& FCustomDemoObjectNodeStaticData::PayLoadKey()
		{
			static FAttributeKey AttributeKey(TEXT("__PayloadKey__"));
			return AttributeKey;
		}
	}//ns Interchange
}//ns UE

UCustomDemoObjectNode::UCustomDemoObjectNode()
{
}

void UCustomDemoObjectNode::InitializeCustomDemoObjectNode(const FString& UniqueID, const FString& DisplayLabel)
{
	InitializeNode(UniqueID, DisplayLabel, EInterchangeNodeContainerType::TranslatedAsset);
}

FString UCustomDemoObjectNode::GetTypeName() const
{
	const FString TypeName = TEXT("CustomDemoObjectNode");
	return TypeName;
}

const TOptional<FString> UCustomDemoObjectNode::GetPayLoadKey() const
{
	FString PayloadKey;

	//PayLoadKey
	{
		if (!Attributes->ContainAttribute(UE::Interchange::FCustomDemoObjectNodeStaticData::PayLoadKey()))
		{
			return TOptional<FString>();
		}
		UE::Interchange::FAttributeStorage::TAttributeHandle<FString> AttributeHandle = Attributes->GetAttributeHandle<FString>(UE::Interchange::FCustomDemoObjectNodeStaticData::PayLoadKey());
		if (!AttributeHandle.IsValid())
		{
			return TOptional<FString>();
		}
		UE::Interchange::EAttributeStorageResult Result = AttributeHandle.Get(PayloadKey);
		if (!IsAttributeStorageResultSuccess(Result))
		{
			LogAttributeStorageErrors(Result, TEXT("UCustomDemoObjectNode.GetPayLoadKey"), UE::Interchange::FCustomDemoObjectNodeStaticData::PayLoadKey());
			return TOptional<FString>();
		}
	}

	return TOptional<FString>(PayloadKey);
}

void UCustomDemoObjectNode::SetPayLoadKey(const FString& PayLoadKey)
{
	UE::Interchange::EAttributeStorageResult Result = Attributes->RegisterAttribute(UE::Interchange::FCustomDemoObjectNodeStaticData::PayLoadKey(), PayLoadKey);
	if (!IsAttributeStorageResultSuccess(Result))
	{
		LogAttributeStorageErrors(Result, TEXT("UCustomDemoObjectNode.SetPayLoadKey"), UE::Interchange::FCustomDemoObjectNodeStaticData::PayLoadKey());
	}
}
