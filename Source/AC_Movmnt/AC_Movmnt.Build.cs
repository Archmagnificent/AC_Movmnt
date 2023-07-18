// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class AC_Movmnt : ModuleRules
{
	public AC_Movmnt(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "EnhancedInput" });
	}
}
