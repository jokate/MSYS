// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/GameplayEffect/YSGE_SwitchEntryGroggy.h"

#include "Character/AttributeSet/YSGroggyAttributeSet.h"
#include "General/YSGameplayTag.h"

UYSGE_SwitchEntryGroggy::UYSGE_SwitchEntryGroggy()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FSetByCallerFloat Damage;
	Damage.DataTag = YSTags::Data_Groggy_SwitchEntryDamage;

	FGameplayModifierInfo Modifier;
	Modifier.Attribute = UYSGroggyAttributeSet::GetIncomingGroggyDamageAttribute();
	Modifier.ModifierOp = EGameplayModOp::Additive;
	Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(Damage);
	Modifiers.Add(Modifier);
}
