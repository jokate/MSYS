// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "General/YSStruct.h"
#include "YSAbilityDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class MNYS_API UYSAbilityDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public : 
	TArray<FYSGrantedAbilityData> GetAllAbilities() const;
	
protected : 
	UPROPERTY(EditDefaultsOnly, Category = "YS | GameplayAbility", meta = (DisplayName = "기본 공격"))
	TArray<FYSGrantedAbilityData> BasicAttackAbilities;
	
	UPROPERTY(EditDefaultsOnly, Category = "YS | GameplayAbility", meta = (DisplayName = "회피"))
	TArray<FYSGrantedAbilityData> AvoidanceAbilities;
	
	UPROPERTY(EditDefaultsOnly, Category = "YS | GameplayAbility", meta = (DisplayName = "스킬"))
	TArray<FYSGrantedAbilityData> SkillAbilities;
	
	UPROPERTY(EditDefaultsOnly, Category = "YS | GameplayAbility", meta = (DisplayName = "공중 공격"))
	TArray<FYSGrantedAbilityData> AirAttackAbilities;
	
	UPROPERTY(EditDefaultsOnly, Category = "YS | GameplayAbility", meta = (DisplayName = "죽음"))
	FYSGrantedAbilityData DeathAbility;
	
	UPROPERTY(EditDefaultsOnly, Category = "YS | GameplayAbility", meta = (DisplayName = "히트"))
	FYSGrantedAbilityData HitAbility;
};
