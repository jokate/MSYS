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

	UAbilitySystemComponent* AppliedToASC = ActiveGEContainer->Owner;

	if ( IsValid(AppliedToASC) == false )
	{
		return;
	}

	// 공격자가 먼저 사라졌으면 받는 쪽 ASC로 컨텍스트를 만든다.
	UAbilitySystemComponent* InstigatorASC = GEContextHandle.GetInstigatorAbilitySystemComponent();
	UAbilitySystemComponent* ContextASC = IsValid(InstigatorASC) ? InstigatorASC : AppliedToASC;

	FGameplayEffectSpecHandle SpecHandle = FGameplayEffectSpecHandle(new FGameplayEffectSpec());
	FGameplayEffectContextHandle EffectContextHandle = ContextASC->MakeEffectContext();
	EffectContextHandle.SetAbility(GEContextHandle.GetAbility());
	EffectContextHandle.AddInstigator(GEContextHandle.GetInstigator(), GEContextHandle.GetInstigator());
	SpecHandle.Data->Initialize(TargetToAdd.GetDefaultObject(), EffectContextHandle, 1);

	SpecHandle.Data->DynamicAssetTags.AppendTags(TargetToAddTag);

	AppliedToASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}
