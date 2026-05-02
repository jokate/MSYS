// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "YSGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class MNYS_API UYSGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public :
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	void PlayMontage();

protected :
	
	UFUNCTION()
	void OnMontagePlayed();
	UFUNCTION()
	void OnMontageInterrupted();	
public :
	UPROPERTY(EditDefaultsOnly, Category = "Montage To Play", meta = (DisplayName = "재생할 몽타주"))
	TObjectPtr<UAnimMontage> TargetToPlayMontage;

};
