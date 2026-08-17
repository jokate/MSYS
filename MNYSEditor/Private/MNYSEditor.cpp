
#include "MNYSEditor.h"
#include "Details/YSAbilityPlaybackDetails.h"
#include "Details/YSAbilityTestCharacterDetails.h"
#include "PropertyEditorModule.h"
#include "Details/YSIAUSCurveDetails.h"
#include "Editor/PropertyEditor/Public/PropertyEditorModule.h"
#include "EdGraphUtilities.h"
#include "Modules/ModuleManager.h"
#include "Playback/YSPlaybackConnectionDrawingPolicy.h"
#include "Playback/YSPlaybackGraphNodeFactory.h"

#define LOCTEXT_NAMESPACE "MNYSEditor"

void FMNYSEditorModule::StartupModule()
{
	// 커스텀 Details 등록
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout("YSCharacterBase", FOnGetDetailCustomizationInstance::CreateStatic(&FYSAbilityTestCharacterDetails::MakeInstance) );
	PropertyModule.RegisterCustomClassLayout("YSGameplayAbility_AIBase", FOnGetDetailCustomizationInstance::CreateStatic(&FYSIAUSCurveDetails::MakeInstance));

	// 그래프가 소유한 플레이백에서만 Transitions 배열을 감춘다. 레거시 인라인 배열은 그대로 둔다.
	PropertyModule.RegisterCustomClassLayout("YSAbilityPlaybackBase", FOnGetDetailCustomizationInstance::CreateStatic(&FYSAbilityPlaybackDetails::MakeInstance));

	PropertyModule.NotifyCustomizationModuleChanged();

	// 플레이백 그래프 노드 외형 등록. 상태·시작·전환이 전용 위젯으로 그려진다.
	PlaybackNodeFactory = MakeShared<FYSPlaybackGraphNodeFactory>();
	FEdGraphUtilities::RegisterVisualNodeFactory(PlaybackNodeFactory);

	// 연결선 정책 등록. 이게 없으면 전환 노드가 선 위가 아니라 선 옆에 따로 놓인다.
	PlaybackConnectionFactory = MakeShared<FYSPlaybackConnectionFactory>();
	FEdGraphUtilities::RegisterVisualPinConnectionFactory(PlaybackConnectionFactory);
}

void FMNYSEditorModule::ShutdownModule()
{
	if (PlaybackConnectionFactory.IsValid())
	{
		FEdGraphUtilities::UnregisterVisualPinConnectionFactory(PlaybackConnectionFactory);
		PlaybackConnectionFactory.Reset();
	}

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
		PropertyModule.UnregisterCustomClassLayout("YSAbilityPlaybackBase");
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMNYSEditorModule, MNYSEditor)
