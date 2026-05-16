// Fill out your copyright notice in the Description page of Project Settings.


#include "Library/YSBlueprintFunctionLibrary.h"

#include "YSDeveloperSettings.h"
#include "Character/AttributeSet/YSCharacterAttributeSetBase.h"

float UYSBlueprintFunctionLibrary::GetFinalDamage(const UYSCharacterAttributeSetBase* Owner,
                                                  const UYSCharacterAttributeSetBase* Target, const FName& SkillID)
{
	const FYSDamageInfo* DamageInfo = UYSDeveloperSettings::GetDamageInfo(SkillID);

	// 데미지 정보가 없으면 의미 X
	if ( DamageInfo == nullptr )
		return 0.f;
	
	// 당장 생각하는 부분은 (공격 데미지 * (공격 계수)  ) * ( 1 - DefenceRate ) + 고정 데미지
	float FinalDamage = ( Owner->GetAttackDmg() * DamageInfo->DamageMultiplier ) * ( 1 - Target->GetDefenseRate()) + DamageInfo->AdditiveTrueDamage;

	// 차후 데미지 증가 버프 혹은 다른 것들이 추가될 경우 해당 부분에 대해서 확장하기로 합니다.
	
	return FinalDamage;
}
