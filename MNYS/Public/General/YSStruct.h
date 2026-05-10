// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "YSStruct.generated.h"

/**
 * 
 */

class UYSGameplayAbility;

USTRUCT(BlueprintType)
struct FYSGrantedAbilityData
{
	GENERATED_BODY()

public : 
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UYSGameplayAbility> AbilityClass;
	
	UPROPERTY(EditDefaultsOnly)
	FGameplayTagContainer InputTag;        // 인풋 바인딩

	UPROPERTY(EditDefaultsOnly)
	int32 Level = 1;
};