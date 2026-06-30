// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/StateTree/Task/YSAIStateTreeTask.h"

#include "StateTreeExecutionContext.h"

EStateTreeRunStatus FYSStateTreeTaskBase::EnterState(FStateTreeExecutionContext& Context,
                                                     const FStateTreeTransitionResult& Transition) const
{
	UInstanceDataType* InstanceData = Context.GetInstanceDataPtr<UInstanceDataType>(*this);
	check(InstanceData);

	return InstanceData->EnterState(Context, Transition);
}

EStateTreeRunStatus FYSStateTreeTaskBase::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	UInstanceDataType* InstanceData = Context.GetInstanceDataPtr<UInstanceDataType>(*this);
	check(InstanceData);

	return InstanceData->Tick(Context, DeltaTime);
}

void FYSStateTreeTaskBase::ExitState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	UInstanceDataType* InstanceData = Context.GetInstanceDataPtr<UInstanceDataType>(*this);
	check(InstanceData);

	InstanceData->ExitState(Context, Transition);
}
