// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "YSPhaseChangeCondition.generated.h"

class AYSCharacterBase;
/**
 * 
 */
USTRUCT(BlueprintType)
struct MNYS_API FYSPhaseChangeConditions
{
	GENERATED_BODY()
	
public : 
	bool CheckCondition(AYSCharacterBase* CharacterBase) const;
public : 
	UPROPERTY(EditAnywhere,  meta = (DisplayName = "페이즈 변환 조건", BaseStruct = "/Script/MNYS.YSPhaseChangeConditionBase", ExcludeBaseStruct))
	TArray<FInstancedStruct> ConditionStructs;
};

USTRUCT()
struct MNYS_API FYSPhaseChangeConditionBase
{
	GENERATED_BODY()
	
public :
	virtual ~FYSPhaseChangeConditionBase() = default;
	virtual bool CheckCondition(AYSCharacterBase* CharacterBase) const
	{
		return true;
	}
};

USTRUCT(BlueprintType)
struct MNYS_API FYSPhaseChangeCondition_HP : public FYSPhaseChangeConditionBase
{
	GENERATED_BODY()
	
public : 
	virtual bool CheckCondition(AYSCharacterBase* CharacterBase) const override;
	
public : 
	UPROPERTY(EditAnywhere, Category = "YS | Condition", meta = (DisplayName = "HP 비율"))
	float HpRatio = 0.f;
};

USTRUCT(BlueprintType)
struct MNYS_API FYSPhaseChangeCondition_WorldTag : public FYSPhaseChangeConditionBase
{
	GENERATED_BODY()
	
public :
	virtual bool CheckCondition(AYSCharacterBase* CharacterBase) const override;
	
public : 
	FGameplayTag TargetTag;
};