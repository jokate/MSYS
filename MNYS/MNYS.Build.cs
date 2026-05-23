// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MNYS : ModuleRules
{
	public MNYS(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", 
			"GameplayAbilities", "AIModule", "MotionWarping", "GameplayTags", "GameplayTasks", "DataRegistry", "DeveloperSettings", "UMG", "Kismet", "AnimGraphRuntime", "Niagara", "LevelSequence", "MovieScene"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { "AnimationLocomotionLibraryRuntime" });
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore",
				"Networking",
				"Sockets",
				"HTTP",
				"Json",
				"JsonUtilities",
				"DeveloperSettings",
				"PhysicsCore",
				"UnrealEd",           // For Blueprint editing
				"BlueprintGraph",     // For K2Node classes (F15-F22)
				"KismetCompiler"      // For Blueprint compilation (F15-F22),
			}
		);
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"EditorScriptingUtilities",
				"EditorSubsystem",
				"Slate",
				"SlateCore",
				"Kismet",
				"Projects",
				"AssetRegistry"
			}
		);

		PublicIncludePaths.AddRange(new string[] {"MNYS"});
		// Uncomment if you are using Slate UI
		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
