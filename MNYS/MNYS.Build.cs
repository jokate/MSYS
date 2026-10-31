// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MNYS : ModuleRules
{
	public MNYS(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput",
			"GameplayAbilities", "AIModule", "MotionWarping", "GameplayTags", "GameplayTasks", "DataRegistry", "DeveloperSettings",
			"UMG", "AnimGraphRuntime", "Niagara", "LevelSequence", "MovieScene", "NavigationSystem", "StateTreeModule", "PhysicsCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { "AnimationLocomotionLibraryRuntime", "GameplayStateTreeModule", "Slate", "SlateCore" });

		PublicIncludePaths.AddRange(new string[] {"MNYS"});

		// GameplayDebugger (Shipping/Test 제외 자동 처리 + WITH_GAMEPLAY_DEBUGGER* define 세팅)
		SetupGameplayDebuggerSupport(Target);

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}

