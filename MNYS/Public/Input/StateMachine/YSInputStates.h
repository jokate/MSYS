// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "General/YSEnum.h"
#include "UObject/Object.h"
#include "YSInputStates.generated.h"

class UYSInputStateMachineComponent;
class UYSAbilitySystemComponent;
class AYSCharacterBase;
/**
 * 해당 시스템은 외부에서 변경 제어되는 StateMachine의 형태이며, 오로지 Input에 대한 제어를 목표로 합니다.
 */
UCLASS()
class MNYS_API UYSInputStates : public UObject
{
	GENERATED_BODY()

public :
	virtual void ProcessInput(const FGameplayTag& InputGameplayTag, EYSInputPhase InputPhase);
	void InitState(AActor* Owner);
	EYSInputStatesType GetStateType() const { return State; }
	
public :
	UPROPERTY()
	FString StateName;
	
	UPROPERTY()
	TWeakObjectPtr<AActor> OwnerActor;
	
	UPROPERTY()
	TWeakObjectPtr<UYSAbilitySystemComponent> OwnerASC;
	
	UPROPERTY()
	TWeakObjectPtr<UYSInputStateMachineComponent> OwnerStateMachineManager;

protected :
	/** "StateName.InputTag" 조합 태그를 해석한다. 최초 1회만 문자열을 만들고 이후엔 캐시를 탄다. */
	FGameplayTag ResolveStateTag(const FGameplayTag& InputGameplayTag);

protected :
	EYSInputStatesType State;

private :
	// InputTag → "StateName.InputTag" 결과 캐시.
	// 무효 태그(미등록 조합)도 그대로 캐시해 재조회를 막는다.
	TMap<FGameplayTag, FGameplayTag> ResolvedTagCache;
};

UCLASS()
class UYSIdleState : public UYSInputStates
{
	GENERATED_BODY()

public :
	UYSIdleState() : UYSInputStates()
	{
		StateName = TEXT("Idle");

		State = EYSInputStatesType::Idle;
	}
};


// 기본 공격. 간단한 스킬. (?) 고민 중.
UCLASS()
class UYSAttackState : public UYSInputStates
{
	GENERATED_BODY()

public :
	UYSAttackState() : UYSInputStates()
	{
		StateName = TEXT("Attack");
		State = EYSInputStatesType::Attack;
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
		State = EYSInputStatesType::JustAvoid;
	}
};

// 특수 스킬. 특수한 상황에서 사용하거나 시퀀스 사용하는 스킬.
UCLASS()
class UYSSkillState : public UYSInputStates
{
	GENERATED_BODY()

public :
	UYSSkillState() : UYSInputStates()
	{
		StateName = TEXT("Skill");
		State = EYSInputStatesType::Skill;
	}
};

UCLASS()
class UYSFallingState : public UYSInputStates
{
	GENERATED_BODY()

public :
	UYSFallingState() : UYSInputStates()
	{
		StateName = TEXT("Falling");
		State = EYSInputStatesType::Falling;
	}
};

UCLASS()
class UYSDodgeState : public UYSInputStates
{
	GENERATED_BODY()

public :
	UYSDodgeState() : UYSInputStates()
	{
		StateName = TEXT("Dodge");
		State = EYSInputStatesType::Dodge;
	}
};

UCLASS()
class UYSReadyState : public UYSInputStates
{
	GENERATED_BODY()

public :
	UYSReadyState() : UYSInputStates()
	{
		StateName = TEXT("Ready");
		State = EYSInputStatesType::Ready;
	}
};

UCLASS()
class UYSJumpAttackState : public UYSInputStates
{
	GENERATED_BODY()

	UYSJumpAttackState() : UYSInputStates()
	{
		StateName = TEXT("JumpAttack");
		State = EYSInputStatesType::JumpAttack;
	}
};

UCLASS()
class UYSDamagedState : public UYSInputStates
{
	GENERATED_BODY()

	UYSDamagedState() : UYSInputStates()
	{
		StateName = TEXT("Damaged");
		State = EYSInputStatesType::Damaged;
	}
};


UCLASS()
class UYSDeathState : public UYSInputStates
{
	GENERATED_BODY()

	UYSDeathState() : UYSInputStates()
	{
		StateName = TEXT("Death");
		State = EYSInputStatesType::Death;
	}
};

UCLASS()
class UYSAimState : public UYSInputStates
{
	GENERATED_BODY()
	
	UYSAimState() : UYSInputStates()
	{
		StateName = TEXT("Aim");
		State = EYSInputStatesType::Aim;
	}
};