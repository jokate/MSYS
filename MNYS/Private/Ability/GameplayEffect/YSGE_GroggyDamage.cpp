// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/GameplayEffect/YSGE_GroggyDamage.h"

#include "Character/AttributeSet/YSGroggyAttributeSet.h"
#include "General/YSGameplayTag.h"

UYSGE_GroggyDamage::UYSGE_GroggyDamage()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FSetByCallerFloat Damage;
	Damage.DataTag = YSTags::Data_Groggy_Damage;

	FGameplayModifierInfo Modifier;
	Modifier.Attribute = UYSGroggyAttributeSet::GetIncomingGroggyDamageAttribute();
	Modifier.ModifierOp = EGameplayModOp::Additive;
	Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(Damage);
	Modifiers.Add(Modifier);
}
