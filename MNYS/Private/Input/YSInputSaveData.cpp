// Fill out your copyright notice in the Description page of Project Settings.


#include "Input/YSInputSaveData.h"

#include "Ability/YSGameplayAbility.h"

FGameplayTagContainer UYSInputSaveData::GetInputTagByAbilityClass(const TSubclassOf<UYSGameplayAbility> AbilityClass)
{
	for (const FYSInputSaveDataMemeber& InputSaveData : InputSaveDatas)
	{
		if ( InputSaveData.AbilityClass == nullptr )
		{
			continue;
		}
		
		if ( InputSaveData.AbilityClass == AbilityClass )
		{
			return InputSaveData.InputGameplayTag;
		}
	}
	
	return FGameplayTagContainer();
}
