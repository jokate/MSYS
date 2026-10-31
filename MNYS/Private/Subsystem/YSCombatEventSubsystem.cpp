// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystem/YSCombatEventSubsystem.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"

UYSCombatEventSubsystem* UYSCombatEventSubsystem::Get(const UObject* WorldContextObject)
{
	if (IsValid(WorldContextObject) == false)
	{
		return nullptr;
	}

	const UWorld* World = WorldContextObject->GetWorld();
	const UGameInstance* GameInstance = IsValid(World) ? World->GetGameInstance() : nullptr;
	return IsValid(GameInstance) ? GameInstance->GetSubsystem<UYSCombatEventSubsystem>() : nullptr;
}
