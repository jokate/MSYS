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

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag InputTag;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag NextComboTag;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayAbility> AbilityClass;
};

USTRUCT(BlueprintType)
struct FYSComboData : public FTableRowBase
{
	GENERATED_BODY()

public :
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag ComboTag;

	// 
	UPROPERTY(EditDefaultsOnly)
	TArray<FYSComboTransition> Transitions;
};