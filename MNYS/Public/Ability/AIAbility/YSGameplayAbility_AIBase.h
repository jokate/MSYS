// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Ability/YSGameplayAbility.h"
#include "YSGameplayAbility_AIBase.generated.h"

class UYSAIAbilityScoreFunctionBase;
struct FYSTargetingActorCollections;
/**
 * 
 */
UCLASS()
class MNYS_API UYSGameplayAbility_AIBase : public UYSGameplayAbility
{
	GENERATED_BODY()
	
public : 
	virtual float GetAbilityUtilityScore(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
public : 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, Category = "YS | Condition", meta = (DisplayName = "AI용 Utility Score"))
	TArray<UYSAIAbilityScoreFunctionBase*> UtilityScore;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | Condition", meta = (DisplayName = "AI용 Utility Score Weight"))
	float BaseUtilityScore = 100.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | Condition", meta = (DisplayName = "AI용 Utility Score Threshold"))
	float UtilityScoreThreshold = 50.f;
	
private : 
	TSharedPtr<FYSTargetingActorCollections> TargetingActorCollections;
};
