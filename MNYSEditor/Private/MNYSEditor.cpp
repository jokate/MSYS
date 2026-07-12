
#include "MNYSEditor.h"
#include "Details/YSAbilityTestCharacterDetails.h"
#include "PropertyEditorModule.h"
#include "Details/YSIAUSCurveDetails.h"
#include "Editor/PropertyEditor/Public/PropertyEditorModule.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "MNYSEditor"

void FMNYSEditorModule::StartupModule()
{
	// 커스텀 Details 등록
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout("YSCharacterBase", FOnGetDetailCustomizationInstance::CreateStatic(&FYSAbilityTestCharacterDetails::MakeInstance) );
	PropertyModule.RegisterCustomClassLayout("YSGameplayAbility_AIBase", FOnGetDetailCustomizationInstance::CreateStatic(&FYSIAUSCurveDetails::MakeInstance));
	PropertyModule.NotifyCustomizationModuleChanged();
}

void FMNYSEditorModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomClassLayout("YSCharacterBase");
		PropertyModule.UnregisterCustomClassLayout("YSGameplayAbility_AIBase");
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMNYSEditorModule, MNYSEditor)