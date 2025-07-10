// Copyright Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class CustomPipelines : ModuleRules
	{
		public CustomPipelines(ReadOnlyTargetRules Target) : base(Target)
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
					"MeshDescription",
					"CustomEngine",
					"CustomFactoryNodes",
					"CustomNodes",
					"StaticMeshDescription"
				}
			);

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"ApplicationCore",
					"InterchangeImport",
					"DeveloperSettings",
					"InputCore",
					"Slate",
					"SlateCore",
				}
			);

			if (Target.Type == TargetType.Editor)
			{
				PrivateDependencyModuleNames.AddRange(
					new string[]
					{
						"MainFrame",
						"UnrealEd",
					}
				);
			}
		}
	}
}
