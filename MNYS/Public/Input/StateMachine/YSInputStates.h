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
	UYSInputStates()
	{
		// 대분류. 세부 상태 다음 순위로 태그 해석에 참여한다.
		// 좁은 것부터 넣는다 — 먼저 맞는 하나가 채택되기 때문이다.
		CategoryNames = { TEXT("Grounded"), TEXT("Actionable"), TEXT("Alive") };
	}

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
	/**
	 * "StateName.InputTag" 와 "Category.InputTag" 후보를 세부 → 대분류 순으로 만든다.
	 * 최초 1회만 문자열을 만들고 이후엔 캐시를 탄다.
	 */
	void ResolveStateTags(const FGameplayTag& InputGameplayTag, TArray<FGameplayTag>& OutTags);

protected :
	// 이 상태가 속한 대분류. 상태마다 좁히거나 비운다.
	UPROPERTY()
	TArray<FString> CategoryNames;

	EYSInputStatesType State;

private :
	// InputTag → 후보 태그 목록 캐시.
	// 빈 결과(미등록 조합)도 그대로 캐시해 재조회를 막는다.
	TMap<FGameplayTag, TArray<FGameplayTag>> ResolvedTagCache;
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
		CategoryNames = { TEXT("Actionable"), TEXT("Alive") };   // 공중
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
		CategoryNames = { TEXT("Actionable"), TEXT("Alive") };   // 공중
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
		CategoryNames = { TEXT("Alive") };                       // 경직 중엔 카운터류만
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
		CategoryNames.Empty();                                   // 사망은 무엇도 받지 않는다
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