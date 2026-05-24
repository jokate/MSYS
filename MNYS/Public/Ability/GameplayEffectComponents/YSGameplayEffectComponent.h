// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectComponent.h"
#include "GameplayEffectTypes.h"
#include "YSGameplayEffectComponent.generated.h"

/**
 * 
 */
UCLASS()
class MNYS_API UYSGameplayEffectComponent : public UGameplayEffectComponent
{
	GENERATED_BODY()
	
public :
	virtual bool OnActiveGameplayEffectAdded(FActiveGameplayEffectsContainer& ActiveGEContainer, FActiveGameplayEffect& ActiveGE) const override;

protected :
	virtual void OnActiveGameplayEffectRemoved(const FGameplayEffectRemovalInfo& GameplayEffectRemovalInfo, FActiveGameplayEffectsContainer* ActiveGEContainer) const {};
};
