
#include "MNYSEditor.h"
#include "Details/YSAbilityTestCharacterDetails.h"
#include "PropertyEditorModule.h"
#include "Details/YSIAUSCurveDetails.h"
#include "Editor/PropertyEditor/Public/PropertyEditorModule.h"
#include "EdGraphUtilities.h"
#include "Modules/ModuleManager.h"
#include "Playback/YSPlaybackGraphNodeFactory.h"

#define LOCTEXT_NAMESPACE "MNYSEditor"

void FMNYSEditorModule::StartupModule()
{
	// 커스텀 Details 등록
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout("YSCharacterBase", FOnGetDetailCustomizationInstance::CreateStatic(&FYSAbilityTestCharacterDetails::MakeInstance) );
	PropertyModule.RegisterCustomClassLayout("YSGameplayAbility_AIBase", FOnGetDetailCustomizationInstance::CreateStatic(&FYSIAUSCurveDetails::MakeInstance));
	PropertyModule.NotifyCustomizationModuleChanged();

	// 플레이백 그래프 노드 외형 등록. 전환 노드가 화살표 위 라벨로 그려진다.
	PlaybackNodeFactory = MakeShared<FYSPlaybackGraphNodeFactory>();
	FEdGraphUtilities::RegisterVisualNodeFactory(PlaybackNodeFactory);
}

void FMNYSEditorModule::ShutdownModule()
{
	if (PlaybackNodeFactory.IsValid())
	{
		FEdGraphUtilities::UnregisterVisualNodeFactory(PlaybackNodeFactory);
		PlaybackNodeFactory.Reset();
	}

	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomClassLayout("YSCharacterBase");
		PropertyModule.UnregisterCustomClassLayout("YSGameplayAbility_AIBase");
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMNYSEditorModule, MNYSEditor)
