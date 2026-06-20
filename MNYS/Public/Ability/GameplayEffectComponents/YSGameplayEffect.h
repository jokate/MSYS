// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "YSGameplayEffect.generated.h"

class UNiagaraComponent;
/**
 * 
 */
UCLASS()
class MNYS_API UYSGameplayEffect : public UGameplayEffect
{
	GENERATED_BODY()
	
public : 
	void AddSpawnedNiagara(UNiagaraComponent* NiagaraComponent);
	
	void ResetAllSpawnedNiagara();
	
	
private : 
	TArray<TWeakObjectPtr<UNiagaraComponent>> SpawnedNiagaraComponent;
};
