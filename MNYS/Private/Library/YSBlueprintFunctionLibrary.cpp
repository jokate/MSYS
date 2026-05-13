// Fill out your copyright notice in the Description page of Project Settings.


#include "Library/YSBlueprintFunctionLibrary.h"

#include "Character/AttributeSet/YSCharacterAttributeSetBase.h"

float UYSBlueprintFunctionLibrary::GetFinalDamage(const UYSCharacterAttributeSetBase* Owner,
                                                  const UYSCharacterAttributeSetBase* Target, const FString& SkillID)
{
	// 당장 생각하는 부분은 (공격 데미지 ) * ( 1 - DefenceRate )
	float FinalDamage = Owner->GetAttackDmg() * ( 1 - Target->GetDefenseRate());

	// 차후 데미지 증가 버프 혹은 다른 것들이 추가될 경우 해당 부분에 대해서 확장하기로 합니다.
	
	return FinalDamage;
}
