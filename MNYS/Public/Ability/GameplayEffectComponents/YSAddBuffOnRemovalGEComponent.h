// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "YSGameplayEffectComponent.h"
#include "YSAddBuffOnRemovalGEComponent.generated.h"

/**
 * 
 */
UCLASS(DisplayName = "버프 제거 시, 버프 추가.")
class MNYS_API UYSAddBuffOnRemovalGEComponent : public UYSGameplayEffectComponent
{
	GENERATED_BODY()
	
protected : 
	virtual void OnActiveGameplayEffectRemoved(const FGameplayEffectRemovalInfo& GameplayEffectRemovalInfo, FActiveGameplayEffectsContainer* ActiveGEContainer) const override;
	
protected : 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<class UYSGameplayEffect> TargetToAdd;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagContainer TargetToAddTag;
};
