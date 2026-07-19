// Copyright Epic Games, Inc. All Rights Reserved.

#include "MNYS.h"

#include "Ability/AIAbility/GameplayDebuggerCategory_IAUS.h"
#include "Modules/ModuleManager.h"

#if WITH_GAMEPLAY_DEBUGGER_MENU
#include "GameplayDebugger.h"

#endif

void FMNYSModule::StartupModule()
{
#if WITH_GAMEPLAY_DEBUGGER_MENU
	IGameplayDebugger& GameplayDebuggerModule = IGameplayDebugger::Get();
	GameplayDebuggerModule.RegisterCategory("YS_IAUS",
		IGameplayDebugger::FOnGetCategory::CreateStatic(&FGameplayDebuggerCategory_IAUS::MakeInstance),
		EGameplayDebuggerCategoryState::EnabledInGameAndSimulate);
	GameplayDebuggerModule.NotifyCategoriesChanged();
#endif
}

void FMNYSModule::ShutdownModule()
{
#if WITH_GAMEPLAY_DEBUGGER_MENU
	if (IGameplayDebugger::IsAvailable())
	{
		IGameplayDebugger& GameplayDebuggerModule = IGameplayDebugger::Get();
		GameplayDebuggerModule.UnregisterCategory("YS_IAUS");
		GameplayDebuggerModule.NotifyCategoriesChanged();
	}
#endif
}

IMPLEMENT_PRIMARY_GAME_MODULE( FMNYSModule, MNYS, "MNYS" );
