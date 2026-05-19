// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/YSAbilityDataAsset.h"

TArray<FYSGrantedAbilityData> UYSAbilityDataAsset::GetAllAbilities() const
{
	TArray<FYSGrantedAbilityData> RetVal;
	
	RetVal.Append(BasicAttackAbilities);
	RetVal.Append(AvoidanceAbilities);
	RetVal.Append(SkillAbilities);
	RetVal.Append(AirAttackAbilities);
	RetVal.Add(DeathAbility);
	
	return RetVal;
}
