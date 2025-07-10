// Copyright Epic Games, Inc. All Rights Reserved.

#include "CustomEngineModule.h"

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogCustomEngine);

class FCustomEngineModule : public ICustomEngineModule
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE(FCustomEngineModule, CustomEngine)



void FCustomEngineModule::StartupModule()
{
	//Register anything needed to the interchange manager
	//UInterchangeManager& InterchangeManager = UInterchangeManager::GetInterchangeManager();
	//InterchangeManager.Register...
}


void FCustomEngineModule::ShutdownModule()
{

}



