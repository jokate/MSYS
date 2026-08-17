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
	bool IsEnableTransition(EYSInputStatesType NextState) const { return TransitionRule.Contains(NextState); }
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

	UPROPERTY()
	TSet<EYSInputStatesType> TransitionRule;

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

		for ( int32 i = 0; i < static_cast<int32>(EYSInputStatesType::End); ++i )
		{
			TransitionRule.Add(static_cast<EYSInputStatesType>(i));
		}

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
		TransitionRule.Add(EYSInputStatesType::Idle);
		TransitionRule.Add(EYSInputStatesType::Skill);
		TransitionRule.Add(EYSInputStatesType::Dodge);
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
		TransitionRule.Add(EYSInputStatesType::Idle);
		TransitionRule.Add(EYSInputStatesType::Skill);
		TransitionRule.Add(EYSInputStatesType::Attack);
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
		TransitionRule.Add(EYSInputStatesType::Idle);
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
		TransitionRule.Add(EYSInputStatesType::Idle);
		TransitionRule.Add(EYSInputStatesType::Attack);
		TransitionRule.Add(EYSInputStatesType::Skill);
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
		TransitionRule.Add(EYSInputStatesType::Idle);
		TransitionRule.Add(EYSInputStatesType::Attack);
		TransitionRule.Add(EYSInputStatesType::Falling);
		TransitionRule.Add(EYSInputStatesType::JustAvoid);
		// 구르기 직후 조준. Idle을 한 번 거치게 만들면 회피-반격 리듬이 한 박자 늦는다.
		TransitionRule.Add(EYSInputStatesType::Aim);
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
		TransitionRule.Add(EYSInputStatesType::Idle);
		TransitionRule.Add(EYSInputStatesType::Skill);
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
		TransitionRule.Add(EYSInputStatesType::Idle);
		TransitionRule.Add(EYSInputStatesType::Falling);
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
		TransitionRule.Add(EYSInputStatesType::Idle);
		TransitionRule.Add(EYSInputStatesType::Falling);
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
		TransitionRule.Add(EYSInputStatesType::Idle);
		TransitionRule.Add(EYSInputStatesType::Falling);
		// 조준 중 회피. Aim.Input.Dodge 태그는 이미 있어 어빌리티는 켜지는데
		// 여기가 비어 있으면 인풋 상태만 Aim에 남아 실제 상태와 어긋난다.
		TransitionRule.Add(EYSInputStatesType::Dodge);
	}
};