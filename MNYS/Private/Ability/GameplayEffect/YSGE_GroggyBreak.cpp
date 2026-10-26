// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/GameplayEffect/YSGE_GroggyBreak.h"

#include "Character/AttributeSet/YSCharacterAttributeSetBase.h"

const FName UYSGE_GroggyBreak::DurationName(TEXT("Groggy.BreakDuration"));
const FName UYSGE_GroggyBreak::DamageTakenMultiplierName(TEXT("Groggy.DamageTakenMultiplier"));

UYSGE_GroggyBreak::UYSGE_GroggyBreak()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;

	FSetByCallerFloat Duration;
	Duration.DataName = DurationName;
	DurationMagnitude = FGameplayEffectModifierMagnitude(Duration);

	FSetByCallerFloat Multiplier;
	Multiplier.DataName = DamageTakenMultiplierName;

	FGameplayModifierInfo DamageTaken;
	DamageTaken.Attribute = UYSCharacterAttributeSetBase::GetIncomingDamageMultiplierAttribute();
	DamageTaken.ModifierOp = EGameplayModOp::MultiplyCompound;
	DamageTaken.ModifierMagnitude = FGameplayEffectModifierMagnitude(Multiplier);
	Modifiers.Add(DamageTaken);
}
