// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Containers/ArrayView.h"
#include "CoreMinimal.h"

namespace UE
{
	namespace Interchange
	{
		struct CUSTOMIMPORT_API FCustomDemoObjectData
		{
			virtual ~FCustomDemoObjectData() = default;
			FCustomDemoObjectData() = default;
			FCustomDemoObjectData(FCustomDemoObjectData&&) = default;
			FCustomDemoObjectData& operator=(FCustomDemoObjectData&&) = default;
			FCustomDemoObjectData(const FCustomDemoObjectData&) = delete;
			FCustomDemoObjectData& operator=(const FCustomDemoObjectData&) = delete;

			FString FormatJsonData;
		};
	}//ns Interchange
}//ns UE


