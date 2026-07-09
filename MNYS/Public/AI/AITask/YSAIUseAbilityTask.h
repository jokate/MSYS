// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Tasks/AITask.h"
#include "YSAIUseAbilityTask.generated.h"

/**
 * 
 */
UCLASS()
class MNYS_API UYSAIUseAbilityTask : public UAITask
{
	GENERATED_BODY()
	
public :
	UYSAIUseAbilityTask(const FObjectInitializer& ObjectInitializer);
	static UYSAIUseAbilityTask* CreateTask(AAIController* Controller);
	void OnAbilityEnded(const FAbilityEndedData& AbilityEndedData);
	virtual void Activate() override;
	virtual void TickTask(float DeltaTime) override;
	
private : 
	void _TryActivateAbility();
	
public : 
	bool bIsActiveAbil = false;
};
