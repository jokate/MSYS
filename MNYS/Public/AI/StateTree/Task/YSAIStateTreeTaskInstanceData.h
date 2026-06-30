// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeExecutionTypes.h"
#include "General/YSEnum.h"
#include "UObject/Object.h"
#include "YSAIStateTreeTaskInstanceData.generated.h"

class AYSAIController;
struct FYSTargetingActorCollections;
class UEnvQuery;
struct FStateTreeExecutionContext;
/**
 * 
 */
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
	// Invalid 한 경우 ( 실패 한 경우에 타겟을 놓을 것인가? 예를 들어서 퍼셉션 된 타겟이 없다면의 가정 조치. )
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