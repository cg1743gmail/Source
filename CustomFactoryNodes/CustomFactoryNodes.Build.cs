// Copyright Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class CustomFactoryNodes : ModuleRules
	{
		public CustomFactoryNodes(ReadOnlyTargetRules Target) : base(Target)
		{
			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"CoreUObject",
                    "CustomEngine",
                    "CustomNodes",
                    "Engine",
					"InterchangeCore",
					"InterchangeEngine",
					"InterchangeNodes",
				}
			);
		}
	}
}
