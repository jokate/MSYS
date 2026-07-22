// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Component/YSPhaseChangeCondition.h"

#include "Character/YSCharacterBase.h"
#include "Subsystem/YSWorldTagSubsystem.h"

bool FYSPhaseChangeConditions::CheckCondition(AYSCharacterBase* CharacterBase) const
{
	for (const FInstancedStruct& Condition : ConditionStructs )
	{
		const FYSPhaseChangeConditions* ConditionPtr = Condition.GetPtr<FYSPhaseChangeConditions>();
		
		if ( ConditionPtr == nullptr )
		{
			continue;
		}
		
		if ( ConditionPtr->CheckCondition(CharacterBase) == false )
		{
			return false;
		}
	}
	
	return true;
}

bool FYSPhaseChangeCondition_HP::CheckCondition(AYSCharacterBase* CharacterBase) const
{
	float CharHpRatio = CharacterBase->GetHpRatio();
	return CharHpRatio <= HpRatio;
}

bool FYSPhaseChangeCondition_WorldTag::CheckCondition(AYSCharacterBase* CharacterBase) const 
{
	UWorld* World = CharacterBase->GetWorld();
	
	if (IsValid(World) == false)
	{
		return false;
	}
	
	UYSWorldTagSubsystem* WorldTagSubsystem = World->GetSubsystem<UYSWorldTagSubsystem>();
	
	if ( IsValid(WorldTagSubsystem) == false )
	{
		return false;
	}
	
	return WorldTagSubsystem->HasWorldTagMatching(TargetTag);
}
