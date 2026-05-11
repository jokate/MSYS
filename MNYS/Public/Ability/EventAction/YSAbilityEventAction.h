// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "General/YSStruct.h"
#include "Input/Combo/YSComboData.h"
#include "UObject/Object.h"
#include "YSAbilityEventAction.generated.h"

struct FYSComboTransition;
struct FGameplayEventData;
class UYSGameplayAbility;
class UGameplayAbility;
/**
 * 어빌리티 이벤트 어셉팅에 따른, 동작 방식에 대해서 정의합니다.
 * 특정 이벤트가 들어온 상황이라면, 각 상황에 맞게 처리가 될 것입니다.
 */
UCLASS(Abstract, EditInlineNew, DefaultToInstanced, BlueprintType, Blueprintable)
class MNYS_API UYSAbilityEventAction : public UObject
{
	GENERATED_BODY()

public :
	UFUNCTION(BlueprintNativeEvent)
	bool Execute(UYSGameplayAbility* OwningAbility, const FGameplayEventData& EventData);
	virtual bool Execute_Implementation(UYSGameplayAbility* OwningAbility, const FGameplayEventData& EventData) { return true; }
};

// 임의 클래스 제작.
UCLASS(DisplayName = "프로젝타일 스폰")
class MNYS_API UYSAbilityEventAction_SpawnProjectile : public UYSAbilityEventAction
{
	GENERATED_BODY()

public :
	virtual bool Execute_Implementation(UYSGameplayAbility* OwningAbility, const FGameplayEventData& EventData) override { return true; }
};

// 트레이스 시작.
UCLASS(DisplayName = "트레이스 시작")
class MNYS_API UYSAbilityEventAction_StartTrace : public UYSAbilityEventAction
{
	GENERATED_BODY()

public :
	virtual bool Execute_Implementation(UYSGameplayAbility* OwningAbility, const FGameplayEventData& EventData) override;
};

UCLASS(DisplayName = "트레이스 종료")
class MNYS_API UYSAbilityEventAction_StopTrace : public UYSAbilityEventAction
{
	GENERATED_BODY()

public :
	virtual bool Execute_Implementation(UYSGameplayAbility* OwningAbility, const FGameplayEventData& EventData) override;	
};