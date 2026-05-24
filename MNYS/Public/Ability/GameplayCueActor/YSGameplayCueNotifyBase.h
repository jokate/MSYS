// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Actor.h"
#include "YSGameplayCueNotifyBase.generated.h"

class UYSGameplayCueActionBase;

UCLASS()
class MNYS_API AYSGameplayCueNotifyBase : public AGameplayCueNotify_Actor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AYSGameplayCueNotifyBase();

protected:
	virtual bool OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	virtual bool OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	
	
public : 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
	TArray<UYSGameplayCueActionBase*> GameplayCueNotifyList;
};
