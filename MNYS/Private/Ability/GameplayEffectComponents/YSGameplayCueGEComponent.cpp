// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/GameplayEffectComponents/YSGameplayCueGEComponent.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"

void UYSGameplayCueGEComponent::OnGameplayEffectApplied(FActiveGameplayEffectsContainer& ActiveGEContainer,
                                                        FGameplayEffectSpec& GESpec, FPredictionKey& PredictionKey) const
{
	Super::OnGameplayEffectApplied(ActiveGEContainer, GESpec, PredictionKey);
	
	UAbilitySystemComponent* ASC = ActiveGEContainer.Owner;
	
	if ( IsValid(ASC) == false )
		return;
	
	for ( const FGameplayTag& Tag : GameplayCueTags )
	{
		 ASC->AddGameplayCue(Tag, GESpec.GetEffectContext());
	}
}

void UYSGameplayCueGEComponent::OnActiveGameplayEffectRemoved(
	const FGameplayEffectRemovalInfo& GameplayEffectRemovalInfo,
	FActiveGameplayEffectsContainer* ActiveGEContainer) const
{
	FScopedActiveGameplayEffectLock ActiveScopeLock(*ActiveGEContainer);

	const FActiveGameplayEffect* ActiveGE = GameplayEffectRemovalInfo.ActiveEffect;
	if (!ensure(ActiveGE))
	{
		return;
	}

	UAbilitySystemComponent* ASC = ActiveGEContainer->Owner;
	if (!ensure(IsValid(ASC)))
	{
		return;
	}
		
	for ( const FGameplayTag& Tag : GameplayCueTags )
	{
		ASC->RemoveGameplayCue(Tag);
	}
}
