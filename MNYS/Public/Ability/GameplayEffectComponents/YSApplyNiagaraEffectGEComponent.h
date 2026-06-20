// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "YSGameplayEffectComponent.h"
#include "YSApplyNiagaraEffectGEComponent.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
/**
 * 
 */
UCLASS(DisplayName = "이펙트 적용 (비주얼)")
class MNYS_API UYSApplyNiagaraEffectGEComponent : public UYSGameplayEffectComponent
{
	GENERATED_BODY()
	
public : 
	virtual void OnGameplayEffectApplied(FActiveGameplayEffectsContainer& ActiveGEContainer, FGameplayEffectSpec& GESpec, FPredictionKey& PredictionKey) const override;
	
protected : 
	virtual void OnActiveGameplayEffectRemoved(const FGameplayEffectRemovalInfo& GameplayEffectRemovalInfo, FActiveGameplayEffectsContainer* ActiveGEContainer) const override;
	
public : 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect")
	TObjectPtr<UNiagaraSystem> TargetToSpawn;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect")
	FName Socket = NAME_None;
	
private : 
	UPROPERTY()
	TWeakObjectPtr<UNiagaraComponent> SpawnedNiagaraComponent;
};
