// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/GameplayEffectComponents/YSInputStateGEComponent.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Input/StateMachine/YSInputStateMachineComponent.h"

void UYSInputStateGEComponent::OnGameplayEffectApplied(FActiveGameplayEffectsContainer& ActiveGEContainer,
                                                       FGameplayEffectSpec& GESpec, FPredictionKey& PredictionKey) const
{
	Super::OnGameplayEffectApplied(ActiveGEContainer, GESpec, PredictionKey);
	
	UAbilitySystemComponent* ASC = ActiveGEContainer.Owner;
	
	if ( IsValid(ASC) == false )
		return;
	
	UYSInputStateMachineComponent* InputStateComponent = UYSInputStateMachineComponent::Get(ASC->GetOwner());
	if ( IsValid(InputStateComponent))
	{
		InputStateComponent->AddStateStack(TargetToChange);
	}
}

void UYSInputStateGEComponent::OnActiveGameplayEffectRemoved(const FGameplayEffectRemovalInfo& GameplayEffectRemovalInfo, FActiveGameplayEffectsContainer* ActiveGEContainer ) const
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

	UYSInputStateMachineComponent* InputStateComponent = UYSInputStateMachineComponent::Get(ASC->GetOwner());
	if ( IsValid(InputStateComponent))
	{
		InputStateComponent->RemoveStateStack(TargetToChange);
	}
}
