// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "YSComboData.generated.h"

class UGameplayAbility;
/**
 * 
 */
USTRUCT(BlueprintType)
struct FYSComboTransition
{
	GENERATED_BODY()

	bool IsTransitionable(const FGameplayTag& InputGameplayTag) const
	{
		return TransitionTags.HasTagExact(InputGameplayTag);
	}
	
	UPROPERTY(EditDefaultsOnly)
	FGameplayTagContainer TransitionTags;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayAbility> AbilityClass;
};

// Purpose : 콤보를 위한 인풋 태그.
USTRUCT(BlueprintType)
struct FYSComboSequence : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag Combo;

	UPROPERTY(EditDefaultsOnly)
	TArray<FGameplayTag> ComboSequence;

	bool IsSatisfiedCombo(const TArray<FGameplayTag>& InputGameplayTags) const;
};