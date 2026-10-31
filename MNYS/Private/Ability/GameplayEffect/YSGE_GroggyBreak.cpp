// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/GameplayEffect/YSGE_GroggyBreak.h"

#include "Character/AttributeSet/YSCharacterAttributeSetBase.h"
#include "General/YSGameplayTag.h"

UYSGE_GroggyBreak::UYSGE_GroggyBreak()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;

	FSetByCallerFloat Duration;
	Duration.DataTag = YSTags::Data_Groggy_BreakDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(Duration);

	FSetByCallerFloat Multiplier;
	Multiplier.DataTag = YSTags::Data_Groggy_DamageTakenMultiplier;

	FGameplayModifierInfo DamageTaken;
	DamageTaken.Attribute = UYSCharacterAttributeSetBase::GetIncomingDamageMultiplierAttribute();
	DamageTaken.ModifierOp = EGameplayModOp::MultiplyCompound;
	DamageTaken.ModifierMagnitude = FGameplayEffectModifierMagnitude(Multiplier);
	Modifiers.Add(DamageTaken);
}
