// Copyright Epic Games, Inc. All Rights Reserved.

#include "CustomImportModule.h"

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "CustomImportLog.h"
#include "InterchangeManager.h"
#include "Misc/CoreDelegates.h"
#include "Modules/ModuleManager.h"
#include "CustomDemoObjectFactory.h"
#include "CustomFooTranslator.h"

DEFINE_LOG_CATEGORY(LogCustomImport);

class FCustomImportModule : public ICustomImportModule
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:

	bool bIsSubstrateEnabled = false;
};

IMPLEMENT_MODULE(FCustomImportModule, CustomImport)



void FCustomImportModule::StartupModule()
{
	auto RegisterItems = []()
	{
		UInterchangeManager& InterchangeManager = UInterchangeManager::GetInterchangeManager();

		//Register the translators
		//Scenes
		InterchangeManager.RegisterTranslator(UCustomFooTranslator::StaticClass());

		//Register the factories
		InterchangeManager.RegisterFactory(UCustomDemoObjectFactory::StaticClass());
	};
	
	if (GEngine)
	{
		RegisterItems();
	}
	else
	{
		FCoreDelegates::OnPostEngineInit.AddLambda(RegisterItems);
	}
}


void FCustomImportModule::ShutdownModule()
{
}



