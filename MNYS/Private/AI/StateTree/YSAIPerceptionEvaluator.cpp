// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/StateTree/YSAIPerceptionEvaluator.h"

#include "StateTreeExecutionContext.h"
#include "AI/YSAIController.h"
#include "EnvironmentQuery/EnvQueryManager.h"

// 베스트 타겟 고정화.
void FYSAIFindTargetEvaluator::TreeStart(FStateTreeExecutionContext& Context) const
{
	if ( Policy != EYSTargetingPolicy::Fixed )
	{
		return;
	}
	
	_SearchBestTarget(Context);
}

void FYSAIFindTargetEvaluator::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	if ( Policy != EYSTargetingPolicy::Dynamic )
	{
		return;
	} 
	
	_SearchBestTarget(Context);
}

void FYSAIFindTargetEvaluator::_SearchBestTarget(const FStateTreeExecutionContext& Context) const
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

	UEnvQueryManager* QueryManager = UEnvQueryManager::GetCurrent(AIController->GetWorld());

	if ( IsValid(QueryManager) == false )
	{
		return;
	} 
	
	FEnvQueryRequest Request(InstanceData.TargetingEQS, AIController);

	TSharedPtr<FEnvQueryResult> Result = QueryManager->RunInstantQuery(Request, EEnvQueryRunMode::Type::SingleResult);

	if ( Result.IsValid() == false )
	{
		if ( NeedToReleaseInvalidResult )
		{
			InstanceData.Reset();
		}
		return;
	}
	
	InstanceData.TargetActor = Result->GetItemAsActor(0);
}


