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

	// 같은 상태를 올린 GE 가 아직 살아 있으면 내리지 않는다. 상태 스택은 요청자를 세지 않아 앞 GE 가 빠질 때 뒤 GE 의 상태까지 내려간다.
	// 지금 빠지는 GE 는 콜백 전에 제거 대기로 표시돼 순회에서 빠진다.
	for ( auto It = ActiveGEContainer->CreateConstIterator(); It; ++It )
	{
		const UYSInputStateGEComponent* Other = It->Spec.Def ? It->Spec.Def->FindComponent<UYSInputStateGEComponent>() : nullptr;

		if ( Other != nullptr && Other->TargetToChange == TargetToChange )
		{
			return;
		}
	}

	UYSInputStateMachineComponent* InputStateComponent = UYSInputStateMachineComponent::Get(ASC->GetOwner());
	if ( IsValid(InputStateComponent))
	{
		InputStateComponent->RemoveStateStack(TargetToChange);
	}
}
