// Copyright Epic Games, Inc. All Rights Reserved.

#include "CustomFactoryNodesModule.h"

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FCustomFactoryNodesModule : public ICustomFactoryNodesModule
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE(FCustomFactoryNodesModule, CustomFactoryNodes)



void FCustomFactoryNodesModule::StartupModule()
{

}


void FCustomFactoryNodesModule::ShutdownModule()
{

}



