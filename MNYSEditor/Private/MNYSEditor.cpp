
#include "MNYSEditor.h"
#include "Details/YSAbilityTestCharacterDetails.h"
#include "PropertyEditorModule.h"
#include "Details/YSIAUSCurveDetails.h"
#include "Character/YSCharacterBase.h"
#include "Ability/AIAbility/YSGameplayAbility_AIBase.h"
#include "EdGraphUtilities.h"
#include "Modules/ModuleManager.h"
#include "Playback/YSPlaybackConnectionDrawingPolicy.h"
#include "Playback/YSPlaybackGraphNodeFactory.h"

#define LOCTEXT_NAMESPACE "MNYSEditor"

void FMNYSEditorModule::StartupModule()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout(AYSCharacterBase::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FYSAbilityTestCharacterDetails::MakeInstance) );
	PropertyModule.RegisterCustomClassLayout(UYSGameplayAbility_AIBase::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FYSIAUSCurveDetails::MakeInstance));

	PropertyModule.NotifyCustomizationModuleChanged();

	PlaybackNodeFactory = MakeShared<FYSPlaybackGraphNodeFactory>();
	FEdGraphUtilities::RegisterVisualNodeFactory(PlaybackNodeFactory);

	// 이게 없으면 전환 노드가 선 위가 아니라 선 옆에 따로 놓인다.
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
		PropertyModule.UnregisterCustomClassLayout(AYSCharacterBase::StaticClass()->GetFName());
		PropertyModule.UnregisterCustomClassLayout(UYSGameplayAbility_AIBase::StaticClass()->GetFName());
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMNYSEditorModule, MNYSEditor)
