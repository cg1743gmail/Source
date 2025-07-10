// Copyright Epic Games, Inc. All Rights Reserved.

#include "CustomPipelinesModule.h"

#include "CoreMinimal.h"
#include "CustomPipelineLog.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogCustomPipeline);

class FCustomPipelinesModule : public ICustomPipelinesModule
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override {}
	virtual void ShutdownModule() override {}
};

IMPLEMENT_MODULE(FCustomPipelinesModule, CustomPipelines)
