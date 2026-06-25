// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/AbilityComponent/YSPlaybackCondition.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Ability/YSGameplayAbility.h"

bool FYSPlaybackCondition_HasGameplayEffect::Evaluate(const TSharedPtr<FYSPlaybackContext>& Context) const
{
	UYSGameplayAbility* Ability = Context->OwnerAbility;
	
	if (IsValid(Ability) == false)
	{
		return false;
	}
	
	UAbilitySystemComponent* ASC =  Ability->GetAbilitySystemComponentFromActorInfo();
	if ( IsValid(ASC) == false )
	{
		return false;
	}
	
	// 만약 존재한다면 이게 있겄지야.
	return ASC->GetGameplayEffectCount(GameplayEffect, nullptr) > 0;
}
