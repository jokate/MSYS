// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "General/YSStruct.h"
#include "UObject/Object.h"
#include "YSComboData.generated.h"

class UGameplayAbility;
/**
 * 
 */

// Purpose : 커맨드 시퀀스.
USTRUCT(BlueprintType)
struct FYSCommandSequence : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, meta = (Categories = "Command"))
	FGameplayTag Command;

	UPROPERTY(EditDefaultsOnly, meta = (Categories = "Input"))
	TArray<FGameplayTag> CommandSequence;

	bool IsSatisfiedCommand(const TArray<FYSTagHistory>& InputGameplayTags) const;
};