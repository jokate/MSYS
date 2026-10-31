// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "YSGameplayEffectComponent.h"
#include "YSApplyNiagaraEffectGEComponent.generated.h"

class UNiagaraSystem;
UCLASS(DisplayName = "이펙트 적용 (비주얼)")
class MNYS_API UYSApplyNiagaraEffectGEComponent : public UYSGameplayEffectComponent
{
	GENERATED_BODY()

public :
	virtual bool OnActiveGameplayEffectAdded(FActiveGameplayEffectsContainer& ActiveGEContainer, FActiveGameplayEffect& ActiveGE) const override;

public :
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect")
	TObjectPtr<UNiagaraSystem> TargetToSpawn;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect")
	FName Socket = NAME_None;
};
