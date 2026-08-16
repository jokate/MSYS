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
				// 플레이백 그래프 에디터용.
				// UE 5.7 기준 SGraphEditor 는 UnrealEd 에 있고(Editor/UnrealEd/Public/GraphEditor.h),
				// GraphEditor 모듈은 노드 위젯·연결선 커스터마이즈에 필요하다.
				"GraphEditor",
				"AssetDefinition",
				"ToolMenus",
			}
		);
	}
}