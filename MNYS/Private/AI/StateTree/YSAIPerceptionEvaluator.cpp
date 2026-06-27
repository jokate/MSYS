// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/StateTree/YSAIPerceptionEvaluator.h"

#include "StateTreeExecutionContext.h"
#include "AI/YSAIController.h"

void FYSAIPerceptionEvaluator::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	UObject* ContextObject = Context.GetOwner();
	if ( IsValid(ContextObject) == false )
	{
		return;
	}

	AYSAIController* AIController = Cast<AYSAIController>(ContextObject);

	if (IsValid(AIController) == false)
	{
		return;
	}
	
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.Reset();

	// 여기서 퍼셉션 타겟 제공
}
