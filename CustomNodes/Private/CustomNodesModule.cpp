// Copyright Epic Games, Inc. All Rights Reserved.

#include "CustomNodesModule.h"

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogCustomNodes);

class FCustomNodesModule : public ICustomNodesModule
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE(FCustomNodesModule, CustomNodes)



void FCustomNodesModule::StartupModule()
{
	//Register anything needed to the interchange manager
	//UInterchangeManager& InterchangeManager = UInterchangeManager::GetInterchangeManager();
	//InterchangeManager.Register...
}


void FCustomNodesModule::ShutdownModule()
{

}



