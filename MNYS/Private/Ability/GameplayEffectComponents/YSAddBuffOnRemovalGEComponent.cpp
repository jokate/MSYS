// Fill out your copyright notice in the Description page of Project Settings.

#include "Ability/GameplayEffectComponents/YSAddBuffOnRemovalGEComponent.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Ability/GameplayEffectComponents/YSGameplayEffect.h"

void UYSAddBuffOnRemovalGEComponent::OnActiveGameplayEffectRemoved(
	const FGameplayEffectRemovalInfo& GameplayEffectRemovalInfo,
	FActiveGameplayEffectsContainer* ActiveGEContainer) const
{
	const FGameplayEffectContextHandle& GEContextHandle = GameplayEffectRemovalInfo.EffectContext;
	
	FGameplayEffectSpecHandle SpecHandle = FGameplayEffectSpecHandle(new FGameplayEffectSpec());
	FGameplayEffectContextHandle EffectContextHandle = GEContextHandle.GetInstigatorAbilitySystemComponent()->MakeEffectContext();
	EffectContextHandle.SetAbility(GEContextHandle.GetAbility());
	EffectContextHandle.AddInstigator(GEContextHandle.GetInstigator(), GEContextHandle.GetInstigator());

	SpecHandle.Data->Initialize(TargetToAdd.GetDefaultObject(), EffectContextHandle, 1);
	
	UAbilitySystemComponent* AppliedToASC = ActiveGEContainer->Owner;;
	
	if ( AppliedToASC )
	{
		AppliedToASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}
