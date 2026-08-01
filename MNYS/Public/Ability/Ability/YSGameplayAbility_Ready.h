// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Ability/YSGameplayAbility.h"
#include "YSGameplayAbility_Ready.generated.h"

class AYSSkillIndicator;
/**
 * 
 */
USTRUCT(BlueprintType)
struct FYSInputPhase
{
	GENERATED_BODY()
	
	bool operator==(const FYSInputPhase& InputPhase) const
	{
		return InputPhase.Tag == Tag && InputPhase.Phase == Phase;
	}
public : 
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag Tag;
	
	UPROPERTY(EditDefaultsOnly)
	EYSInputPhase Phase;
};

UCLASS()
class MNYS_API UYSGameplayAbility_Ready : public UYSGameplayAbility
{
	GENERATED_BODY()
	
public : 
	
	virtual bool TryTransition(const FGameplayTag& InputGameplayTag, EYSInputPhase InputPhase) override;
	
public : 
	// 태그 처리 중에 Cancel 태그가 들어오면 Ready 상태를 해제합니다.
	UPROPERTY(EditDefaultsOnly, Category = "YS | Ready Cancel")
	FYSInputPhase ReadyCancelInfo;
	
};
