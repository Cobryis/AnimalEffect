// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class AnimalEffect : ModuleRules
{
	public AnimalEffect(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] 
		{ 
			"Core", "CoreUObject", "Engine", "InputCore",
			"UMG"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		PublicIncludePaths.Add(ModuleDirectory);
	}
}
