// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Input/Combo/YSComboData.h"
#include "YSGameplayAbility.generated.h"

struct FYSComboTransition;
/**
 * 
 */
UCLASS()
class MNYS_API UYSGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

	public :
		virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	bool TryTransition(const FGameplayTag& InputGameplayTag) const;
	
protected :
	UFUNCTION()
	void OnMontagePlayed();

	UFUNCTION()
	void OnMontageInterrupted();
	
	void PlayMontage();
	
public :
	UPROPERTY(EditDefaultsOnly, Category = "Montage To Play", meta = (DisplayName = "재생할 몽타주"))
	TObjectPtr<UAnimMontage> TargetToPlayMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Input Related", meta = (DisplayName = "인풋에 따른 반응"))
	TArray<FYSComboTransition> TransitionsByInput;
};
