// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "YSGameplayEffectComponent.h"
#include "YSLockOnGEComponent.generated.h"

/**
 * 
 */
UCLASS(DisplayName = "버프 대상 락온 (플레이어 한정)")
class MNYS_API UYSLockOnGEComponent : public UYSGameplayEffectComponent
{
	GENERATED_BODY()
	
public : 
	virtual void OnActiveGameplayEffectRemoved(const FGameplayEffectRemovalInfo& GameplayEffectRemovalInfo, FActiveGameplayEffectsContainer* ActiveGEContainer) const override;
	virtual void OnGameplayEffectApplied(FActiveGameplayEffectsContainer& ActiveGEContainer, FGameplayEffectSpec& GESpec, FPredictionKey& PredictionKey) const override;
};
