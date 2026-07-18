// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "YSAT_RunEQSQuery.generated.h"

class UYSGameplayAbility;
class UEnvQuery;
/**
 * 
 */

DECLARE_DELEGATE_TwoParams(FYSEnvQueryResultDelegate, UYSGameplayAbility*, FVector);

UCLASS()
class MNYS_API UYSAT_RunEQSQuery : public UAbilityTask
{
	GENERATED_BODY()
	
public : 
	static UYSAT_RunEQSQuery* CreateTask(UGameplayAbility* OwningAbility, UEnvQuery* QueryTemplate);
	
	virtual void Activate() override;
	virtual void OnDestroy(bool bInOwnerFinished) override;
	
public : 
	FYSEnvQueryResultDelegate OnSucceeded;

private : 
	UPROPERTY()
	UEnvQuery* QueryToActive;
};
