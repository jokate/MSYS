// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/GameplayEffect/YSGE_ContextSwitchStagger.h"

const FName UYSGE_ContextSwitchStagger::DurationName(TEXT("ContextSwitch.StaggerDuration"));

UYSGE_ContextSwitchStagger::UYSGE_ContextSwitchStagger()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;

	FSetByCallerFloat Duration;
	Duration.DataName = DurationName;
	DurationMagnitude = FGameplayEffectModifierMagnitude(Duration);
}
