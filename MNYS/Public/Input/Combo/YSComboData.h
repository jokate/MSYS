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

	// 즉시 반응할거냐 아니면 윈도우에 의한 펜딩 처리 ( 콤보죠? )
	UPROPERTY(EditDefaultsOnly)
	bool bNeedPending = false;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayAbility> AbilityClass;
};

// Purpose : 커맨드 시퀀스.
USTRUCT(BlueprintType)
struct FYSCommandSequence : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, meta = (Categories = "Command"))
	FGameplayTag Command;

	UPROPERTY(EditDefaultsOnly, meta = (Categories = "Input"))
	TArray<FGameplayTag> CommandSequence;

	bool IsSatisfiedCommand(const TArray<FGameplayTag>& InputGameplayTags) const;
};