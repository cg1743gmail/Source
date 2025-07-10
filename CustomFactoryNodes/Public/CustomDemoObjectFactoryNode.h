// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Nodes/InterchangeFactoryBaseNode.h"

#include "CoreMinimal.h"
#include "DemoObject.h"

#include "CustomDemoObjectFactoryNode.generated.h"


UCLASS(BlueprintType)
class CUSTOMFACTORYNODES_API UCustomDemoObjectFactoryNode : public UInterchangeFactoryBaseNode
{
	GENERATED_BODY()

public:
	virtual UClass* GetObjectClass() const override
	{
		return UDemoObject::StaticClass();
	}
	
	/** The Payload Key */
	virtual const TOptional<FString> GetPayLoadKey() const
	{
		FString PayloadKey;
		//PayLoadKey
		{
			if (!Attributes->ContainAttribute(Macro_CustomPayLoadKeyKey))
			{
				return TOptional<FString>();
			}
			UE::Interchange::FAttributeStorage::TAttributeHandle<FString> AttributeHandle = Attributes->GetAttributeHandle<FString>(Macro_CustomPayLoadKeyKey);
			if (!AttributeHandle.IsValid())
			{
				return TOptional<FString>();
			}
			UE::Interchange::EAttributeStorageResult Result = AttributeHandle.Get(PayloadKey);
			if (!IsAttributeStorageResultSuccess(Result))
			{
				LogAttributeStorageErrors(Result, TEXT("UDemoObjectFactoryNode.GetPayLoadKey"), Macro_CustomPayLoadKeyKey);
				return TOptional<FString>();
			}
		}
		return TOptional<FString>(PayloadKey);
	}

	UFUNCTION(BlueprintCallable, Category = "Interchange | Node | DemoObject")
	virtual bool SetPayLoadKey(const FString& AttributeValue)
	{
		IMPLEMENT_NODE_ATTRIBUTE_SETTER_NODELEGATE(PayLoadKey, FString)
	}

	UFUNCTION(BlueprintCallable, Category = "Interchange | Node | DemoObject")
	bool GetCustombAssetSettingBoolean(bool& AttributeValue) const
	{
		IMPLEMENT_NODE_ATTRIBUTE_GETTER(bAssetSettingBoolean, bool)
	}

	UFUNCTION(BlueprintCallable, Category = "Interchange | Node | DemoObject")
	bool SetCustombAssetSettingBoolean(const bool AttributeValue, bool bAddApplyDelegate)
	{
		IMPLEMENT_NODE_ATTRIBUTE_SETTER(UObjectFactoryNode, bAssetSettingBoolean, bool, UDemoObject)
	}

	UFUNCTION(BlueprintCallable, Category = "Interchange | Node | DemoObject")
	bool GetCustomAssetSettingFloat(float& AttributeValue) const
	{
		IMPLEMENT_NODE_ATTRIBUTE_GETTER(AssetSettingFloat, float)
	}

	UFUNCTION(BlueprintCallable, Category = "Interchange | Node | DemoObject")
	bool SetCustomAssetSettingFloat(const float AttributeValue, bool bAddApplyDelegate)
	{
		IMPLEMENT_NODE_ATTRIBUTE_SETTER(UDemoObjectFactoryNode, AssetSettingFloat, float, UDemoObject)
	}

	UFUNCTION(BlueprintCallable, Category = "Interchange | Node | DemoObject")
	bool GetCustomAssetSettingString(FString& AttributeValue) const
	{
		IMPLEMENT_NODE_ATTRIBUTE_GETTER(AssetSettingString, FString)
	}

	UFUNCTION(BlueprintCallable, Category = "Interchange | Node | DemoObject")
	bool SetCustomAssetSettingString(const FString AttributeValue, bool bAddApplyDelegate)
	{
		IMPLEMENT_NODE_ATTRIBUTE_SETTER(UDemoObjectFactoryNode, AssetSettingString, FString, UDemoObject)
	}

	UFUNCTION(BlueprintCallable, Category = "Interchange | Node | DemoObject")
	bool GetCustomAssetSettingEnum(EDemoObjectUnit& AttributeValue) const
	{
		IMPLEMENT_NODE_ATTRIBUTE_GETTER(AssetSettingEnum, EDemoObjectUnit)
	}

	UFUNCTION(BlueprintCallable, Category = "Interchange | Node | DemoObject")
	bool SetCustomAssetSettingEnum(const EDemoObjectUnit AttributeValue, bool bAddApplyDelegate)
	{
		IMPLEMENT_NODE_ATTRIBUTE_SETTER(UDemoObjectFactoryNode, AssetSettingEnum, EDemoObjectUnit, UDemoObject)
	}

private:
	IMPLEMENT_NODE_ATTRIBUTE_KEY(PayLoadKey);
	IMPLEMENT_NODE_ATTRIBUTE_KEY(bAssetSettingBoolean);
	IMPLEMENT_NODE_ATTRIBUTE_KEY(AssetSettingFloat);
	IMPLEMENT_NODE_ATTRIBUTE_KEY(AssetSettingString);
	IMPLEMENT_NODE_ATTRIBUTE_KEY(AssetSettingEnum);
};