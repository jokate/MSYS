
#include "MNYSEditor.h"
#include "Details/YSAbilityTestCharacterDetails.h"
#include "PropertyEditorModule.h"
#include "Editor/PropertyEditor/Public/PropertyEditorModule.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "MNYSEditor"

void FMNYSEditorModule::StartupModule()
{
	// 커스텀 Details 등록
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout(
		"YSCharacterBase",
		FOnGetDetailCustomizationInstance::CreateStatic(&FYSAbilityTestCharacterDetails::MakeInstance)
	);
}

void FMNYSEditorModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomClassLayout("YSCharacterBase");
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMNYSEditorModule, MNYSEditor)