// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/GameplayEffectComponents/YSLockOnGEComponent.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Character/Components/YSCameraManageComponent.h"

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

	UYSCameraManageComponent* LockOnComponent = UYSCameraManageComponent::Get(ASC->GetOwner());
	
	if ( IsValid(LockOnComponent))
	{
		LockOnComponent->ReleaseLockOn();
	}
}

void UYSLockOnGEComponent::OnGameplayEffectApplied(FActiveGameplayEffectsContainer& ActiveGEContainer,
	FGameplayEffectSpec& GESpec, FPredictionKey& PredictionKey) const
{
	Super::OnGameplayEffectApplied(ActiveGEContainer, GESpec, PredictionKey);
	
	UAbilitySystemComponent* ASC = ActiveGEContainer.Owner;
	
	if ( IsValid(ASC) == false )
		return;
	
	UYSCameraManageComponent* LockOnComponent = UYSCameraManageComponent::Get(ASC->GetOwner());
	
	const FGameplayEffectContextHandle& EffectContext = GESpec.GetEffectContext();
	if ( IsValid(LockOnComponent))
	{
		LockOnComponent->ForceSetLockOn(EffectContext.GetInstigator());
	}
}
