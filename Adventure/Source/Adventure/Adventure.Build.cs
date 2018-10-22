// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Adventure : ModuleRules
{
	public Adventure(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "UMG",
            "OnlineSubsystem", "OnlineSubsystemSteam", "MoviePlayer" });

        DynamicallyLoadedModuleNames.Add("OnlineSubsystemNull");
    }
}
