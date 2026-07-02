// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/YSAbilityDataAsset.h"

#include "YSAbilitySystemComponent.h"

UYSAbilityDataAsset* UYSAbilityDataAsset::GetDataAssetFromAbilitySystemComponent(UYSAbilitySystemComponent* InASC)
{
	return InASC->GrantAbilityData;
}

TArray<FYSGrantedAbilityData> UYSAbilityDataAsset::GetAllAbilities() const
{
	TArray<FYSGrantedAbilityData> RetVal;
	
	RetVal.Append(BasicAttackAbilities);
	RetVal.Append(AvoidanceAbilities);
	RetVal.Append(SkillAbilities);
	RetVal.Append(AirAttackAbilities);
	RetVal.Add(DeathAbility);
	RetVal.Add(HitAbility);
	
	return RetVal;
}
