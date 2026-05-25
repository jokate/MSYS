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
	void AddInputSaveData(const FYSInputSaveDataMemeber& Data)
	{
		InputSaveDatas.Add(Data);
	}
	
	FGameplayTagContainer GetInputTagByAbilityClass(const TSubclassOf<UYSGameplayAbility> AbilityClass);
	
	
protected : 
	UPROPERTY()
	TArray<FYSInputSaveDataMemeber> InputSaveDatas;
};
