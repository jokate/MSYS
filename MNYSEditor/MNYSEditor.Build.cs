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

		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Engine",
				"MNYS",
				"GameplayAbilities",
				"Slate",
				"SlateCore",
				"UnrealEd",
				"PropertyEditor",
				"DetailCustomizations",
				// 플레이백 그래프의 노드 위젯 · 연결선 커스터마이즈용.
				"GraphEditor",
				"AssetDefinition",
				"ToolMenus",
				"DataValidation",
				"GameplayTags",
				"Json",
			}
		);
	}
}