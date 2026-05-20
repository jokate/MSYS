using UnrealBuildTool;

public class MNYSEditor : ModuleRules
{
	public MNYSEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
				"MNYSEditor/Public",
				"MNYSEditor"
			}
		);

		PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"MNYS",
			}
		);
		
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", 
			"GameplayAbilities", "AIModule", "MotionWarping", "GameplayTags", "GameplayTasks", "DataRegistry", "DeveloperSettings", "UMG", "Kismet", "AnimGraphRuntime" });


		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"UnrealEd",
				"PropertyEditor",
				"DetailCustomizations",
			}
		);
	}
}