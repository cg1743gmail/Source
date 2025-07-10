// Copyright Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class CustomImport : ModuleRules
	{
		public CustomImport(ReadOnlyTargetRules Target) : base(Target)
		{
			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"CoreUObject",
					"Engine",
					"InterchangeCore",
					"InterchangeEngine",
					"InterchangeFactoryNodes",
					"InterchangeNodes",
					"CustomEngine",
					"CustomFactoryNodes",
					"CustomNodes"
				}
			);
			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"InterchangeCommonParser",
					"InterchangeMessages",
					"Json",
				}
			);
		}
	}
}
