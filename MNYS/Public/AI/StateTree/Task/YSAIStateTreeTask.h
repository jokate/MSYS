// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "YSAIStateTreeTaskInstanceData.h"
#include "UObject/Object.h"
#include "YSAIStateTreeTask.generated.h"

/**
 * 
 */

USTRUCT()
struct MNYS_API FYSStateTreeTaskBase : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()
	
public : 
	using UInstanceDataType = UYSAIStateTreeTaskInstanceData;
	
	virtual const UStruct* GetInstanceDataType() const override { return UInstanceDataType::StaticClass(); }
	
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};

USTRUCT(DisplayName = "타겟팅 처리")
struct MNYS_API FYSTargetingTask : public FYSStateTreeTaskBase
{
	GENERATED_BODY()
	
public :
	using UInstanceDataType = UYSAIStateTreeTask_TargetActor;
	virtual const UStruct* GetInstanceDataType() const override { return UInstanceDataType::StaticClass(); }
};

USTRUCT(DisplayName = "어빌리티 사용")
struct MNYS_API FYSUseAbilityTask : public FYSStateTreeTaskBase
{
	GENERATED_BODY()

public :
	using UInstanceDataType = UYSAIStateTreeTask_UseAbility;
	virtual const UStruct* GetInstanceDataType() const override { return UInstanceDataType::StaticClass(); }
};

USTRUCT(DisplayName = "타겟 주시 (이 상태 동안만)")
struct MNYS_API FYSFocusTargetTask : public FYSStateTreeTaskBase
{
	GENERATED_BODY()

public :
	using UInstanceDataType = UYSAIStateTreeTask_FocusTarget;
	virtual const UStruct* GetInstanceDataType() const override { return UInstanceDataType::StaticClass(); }
};