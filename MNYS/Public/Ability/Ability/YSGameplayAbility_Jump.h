// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Ability/YSGameplayAbility.h"
#include "YSGameplayAbility_Jump.generated.h"

/**
 * 
 */
UCLASS()
class MNYS_API UYSGameplayAbility_Jump : public UYSGameplayAbility
{
	GENERATED_BODY()
	
public : 
	UYSGameplayAbility_Jump();
	
protected :
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
