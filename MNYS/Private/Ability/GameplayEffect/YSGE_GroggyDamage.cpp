// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/GameplayEffect/YSGE_GroggyDamage.h"

#include "Character/AttributeSet/YSGroggyAttributeSet.h"

const FName UYSGE_GroggyDamage::GroggyDamageName(TEXT("Groggy.Damage"));

UYSGE_GroggyDamage::UYSGE_GroggyDamage()
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
