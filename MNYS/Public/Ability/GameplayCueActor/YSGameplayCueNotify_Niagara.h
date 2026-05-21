// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Actor.h"
#include "YSGameplayCueNotify_Niagara.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;

UCLASS()
class MNYS_API AYSGameplayCueNotify_Niagara : public AGameplayCueNotify_Actor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AYSGameplayCueNotify_Niagara();
	
	virtual bool OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	 
protected:
	// 에셋은 Soft로 잡아서 로딩 부담 분리
	UPROPERTY(EditDefaultsOnly, Category = "YS | FX", meta = (DisplayName = "나이아가라 이펙트"))
	TSoftObjectPtr<UNiagaraSystem> NiagaraEffect;
	
	UPROPERTY(EditDefaultsOnly, Category = "YS | Audio", meta = (DisplayName = "활성화 사운드"))
	TSoftObjectPtr<USoundBase> ActivateSound;
private:
	UPROPERTY()
	TObjectPtr<UNiagaraComponent> SpawnedFX;
};
