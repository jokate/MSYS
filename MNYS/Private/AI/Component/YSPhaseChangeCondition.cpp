// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Component/YSPhaseChangeCondition.h"

#include "Character/YSCharacterBase.h"
#include "Subsystem/YSWorldTagSubsystem.h"

bool FYSPhaseChangeConditions::CheckCondition(AYSCharacterBase* CharacterBase) const
{
	for (int32 SlotIndex = 0; SlotIndex < ConditionStructs.Num(); ++SlotIndex )
	{
		const FYSPhaseChangeConditionBase* ConditionPtr = ConditionStructs[SlotIndex].GetPtr<FYSPhaseChangeConditionBase>();

		if ( ConditionPtr == nullptr )
		{
			if ( bWarnedInvalidSlot == false )
			{
				bWarnedInvalidSlot = true;
				UE_LOG(LogTemp, Warning, TEXT("[PhaseChange] %s 의 조건 %d번 칸이 비었거나 조건 형이 아니다. 거짓으로 판정한다."), *GetNameSafe(CharacterBase), SlotIndex);
			}
			return false;
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
