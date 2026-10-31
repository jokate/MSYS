// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/GameplayEffectComponents/YSLockOnGEComponent.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Character/Components/YSCombatTargetComponent.h"

void UYSLockOnGEComponent::OnActiveGameplayEffectRemoved(const FGameplayEffectRemovalInfo& GameplayEffectRemovalInfo,
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

	if ( UYSCombatTargetComponent* CombatTarget = UYSCombatTargetComponent::Get(ASC->GetOwner()) )
	{
		CombatTarget->ReleaseForcedLockOn();
	}
}

void UYSLockOnGEComponent::OnGameplayEffectApplied(FActiveGameplayEffectsContainer& ActiveGEContainer,
	FGameplayEffectSpec& GESpec, FPredictionKey& PredictionKey) const
{
	Super::OnGameplayEffectApplied(ActiveGEContainer, GESpec, PredictionKey);
	
	UAbilitySystemComponent* ASC = ActiveGEContainer.Owner;
	
	if ( IsValid(ASC) == false )
		return;
	
	if ( UYSCombatTargetComponent* CombatTarget = UYSCombatTargetComponent::Get(ASC->GetOwner()) )
	{
		CombatTarget->ForceLockOn(GESpec.GetEffectContext().GetInstigator());
	}
}
