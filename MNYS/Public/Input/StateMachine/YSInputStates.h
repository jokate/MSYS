// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "YSInputStates.generated.h"

class UYSAbilitySystemComponent;
struct FGameplayTag;
class AYSCharacterBase;
/**
 * 해당 시스템은 외부에서 변경 제어되는 StateMachine의 형태이며, 오로지 Input에 대한 제어를 목표로 합니다.
 */
UCLASS()
class MNYS_API UYSInputStates : public UObject
{
	GENERATED_BODY()

public :
	virtual void ProcessInput(const FGameplayTag& InputGameplayTag);
	
public :
	UPROPERTY()
	FString StateName;
	
	UPROPERTY()
	TWeakObjectPtr<AActor> OwnerActor;
	
	UPROPERTY()
	TWeakObjectPtr<UYSAbilitySystemComponent> OwnerASC;
};

UCLASS()
class UYSIdleState : public UYSInputStates
{
	GENERATED_BODY()

public :
	UYSIdleState() : UYSInputStates()
	{
		StateName = TEXT("Idle");
	}
};

UCLASS()
class UYSDashState : public UYSInputStates
{
	GENERATED_BODY()

public :
	UYSDashState() : UYSInputStates()
	{
		StateName = TEXT("Dash");
	}
};

UCLASS()
class UYSAttackState : public UYSInputStates
{
	GENERATED_BODY()

public :
	UYSAttackState() : UYSInputStates()
	{
		StateName = TEXT("Attack");
	}
};

UCLASS()
class UYSJustAvoidState : public UYSInputStates
{
	GENERATED_BODY()

public :
	UYSJustAvoidState() : UYSInputStates()
	{
		StateName = TEXT("JustAvoid");
	}
};