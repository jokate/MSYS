// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "General/YSStruct.h"
#include "YSInputSaveData.generated.h"

/**
 * 
 */


UCLASS()
class MNYS_API UYSInputSaveData : public USaveGame
{
	GENERATED_BODY()
	
public : 
	/** 같은 어빌리티 클래스 항목이 있으면 바꾸고 없으면 더한다. 이번 스쿼드에 없는 캐릭터의 바인딩은 남는다. */
	void AddInputSaveData(const FYSInputSaveDataMemeber& Data);
	
	FGameplayTagContainer GetInputTagByAbilityClass(const TSubclassOf<UYSGameplayAbility> AbilityClass);
	
	
protected : 
	UPROPERTY()
	TArray<FYSInputSaveDataMemeber> InputSaveDatas;
};
