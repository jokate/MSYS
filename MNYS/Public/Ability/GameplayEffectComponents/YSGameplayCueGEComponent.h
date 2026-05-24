// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Ability/GameplayEffectComponents/YSGameplayEffectComponent.h"
#include "YSGameplayCueGEComponent.generated.h"

/**
 * 
 */
UCLASS(DisplayName = "게임 플레이 큐 액터 트리거.")
class MNYS_API UYSGameplayCueGEComponent : public UYSGameplayEffectComponent
{
	GENERATED_BODY()
	
public :
	virtual void OnGameplayEffectApplied(FActiveGameplayEffectsContainer& ActiveGEContainer, FGameplayEffectSpec& GESpec, FPredictionKey& PredictionKey) const override;
protected:
	virtual void OnActiveGameplayEffectRemoved(const FGameplayEffectRemovalInfo& GameplayEffectRemovalInfo, FActiveGameplayEffectsContainer* ActiveGEContainer) const override;

public :
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameplayEffects")
	FGameplayTagContainer GameplayCueTags;
};
