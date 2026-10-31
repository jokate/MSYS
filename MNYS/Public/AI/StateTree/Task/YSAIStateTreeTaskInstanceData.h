// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "StateTreeExecutionTypes.h"
#include "General/YSEnum.h"
#include "UObject/Object.h"
#include "YSAIStateTreeTaskInstanceData.generated.h"

class AYSAIController;
class UAbilitySystemComponent;
struct FAbilityEndedData;
struct FYSTargetingActorCollections;
class UEnvQuery;
struct FStateTreeExecutionContext;
UCLASS()
class MNYS_API UYSAIStateTreeTaskInstanceData : public UObject
{
	GENERATED_BODY()
	
public : 
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
	{
		return EStateTreeRunStatus::Succeeded;
	};
	
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
	{
		return EStateTreeRunStatus::Running;
	}
	
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) {};
	
protected : 
	virtual AYSAIController* GetAIControllerFromContext(const FStateTreeExecutionContext& Context) const;
};


UCLASS()
class MNYS_API UYSAIStateTreeTask_TargetActor : public UYSAIStateTreeTaskInstanceData
{
	GENERATED_BODY()
	
public : 
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;

private : 
	void _SearchBestTarget(const FStateTreeExecutionContext& Context) const;
	
public : 
	// 검색이 실패했을 때(예: 퍼셉션된 타겟 없음) 기존 타겟을 놓을지 여부.
	UPROPERTY(EditAnywhere, Category = "Input | Parameters")
	bool NeedToReleaseInvalidResult = false;
	
	UPROPERTY(EditAnywhere, Category = "Input | Parameters")
	EYSTargetingPolicy Policy = EYSTargetingPolicy::Dynamic;
	
	UPROPERTY(EditAnywhere, Category = "Input | Parameters", meta = (EditCondition = "Policy == EYSTargetingPolicy::Dynamic", EditConditionHides))
	float SearchInterval = 0.5f;
	
	UPROPERTY(EditAnywhere, Category = "Input | Parameters")
	TObjectPtr<UEnvQuery> TargetingEQS;
	
private :
	TSharedPtr<FYSTargetingActorCollections> TargetingActorCollections;
	float CurrentSearchTime = 0.f;
};


UCLASS()
class MNYS_API UYSAIStateTreeTask_FocusTarget : public UYSAIStateTreeTaskInstanceData
{
	GENERATED_BODY()

public :
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;

private :
	AActor* GetFocusTarget(const FStateTreeExecutionContext& Context) const;

public :
	// 매 틱 베스트 타겟을 다시 조회해 주시 대상을 갱신할지 여부 (false면 진입 시점 타겟에 고정)
	UPROPERTY(EditAnywhere, Category = "Input | Parameters", meta = (DisplayName = "타겟을 매 틱 갱신"))
	bool bUpdateEveryTick = true;
};


UCLASS()
class MNYS_API UYSAIStateTreeTask_UseAbility : public UYSAIStateTreeTaskInstanceData
{
	GENERATED_BODY()

public :
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;

private :
	void OnAbilityEnded(const FAbilityEndedData& AbilityEndedData);
	void UnbindAbilityEnded();

protected : 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Parameter)
	FGameplayAbilitySpecHandle AbilityToUse;
	
private :
	TWeakObjectPtr<UAbilitySystemComponent> BoundASC;
	FDelegateHandle AbilityEndedHandle;
	bool bAbilityEnded = false;
};
