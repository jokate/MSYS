// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MNYS : ModuleRules
{
	public MNYS(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", 
			"GameplayAbilities", "AIModule", "MotionWarping", "GameplayTags", "GameplayTasks", "DataRegistry", "DeveloperSettings", "UMG", "Kismet", "AnimGraphRuntime" });

		PrivateDependencyModuleNames.AddRange(new string[] { "AnimationLocomotionLibraryRuntime" });

		PublicIncludePaths.AddRange(new string[] {"MNYS"});
		// Uncomment if you are using Slate UI
		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
