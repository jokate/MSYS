// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "YSGameplayEffectComponent.h"
#include "YSInputStateGEComponent.generated.h"

enum class EYSInputStatesType : uint8;
/**
 * 
 */
UCLASS(DisplayName = "입력 State 전환 (플레이어 한정)")
class MNYS_API UYSInputStateGEComponent : public UYSGameplayEffectComponent
{
	GENERATED_BODY()
	
public : 
	virtual void OnGameplayEffectApplied(FActiveGameplayEffectsContainer& ActiveGEContainer, FGameplayEffectSpec& GESpec, FPredictionKey& PredictionKey) const override;
	
protected : 
	virtual void OnActiveGameplayEffectRemoved(const FGameplayEffectRemovalInfo& GameplayEffectRemovalInfo, FActiveGameplayEffectsContainer* ActiveGEContainer) const override;
	
public :
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, DisplayName = "변경할 State")
	EYSInputStatesType TargetToChange;
};
