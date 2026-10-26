// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/GameplayEffect/YSGE_SwitchEntryGroggy.h"

#include "Character/AttributeSet/YSGroggyAttributeSet.h"

const FName UYSGE_SwitchEntryGroggy::GroggyDamageName(TEXT("Groggy.SwitchEntryDamage"));

UYSGE_SwitchEntryGroggy::UYSGE_SwitchEntryGroggy()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FSetByCallerFloat Damage;
	Damage.DataName = GroggyDamageName;

	FGameplayModifierInfo Modifier;
	Modifier.Attribute = UYSGroggyAttributeSet::GetIncomingGroggyDamageAttribute();
	Modifier.ModifierOp = EGameplayModOp::Additive;
	Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(Damage);
	Modifiers.Add(Modifier);
}
