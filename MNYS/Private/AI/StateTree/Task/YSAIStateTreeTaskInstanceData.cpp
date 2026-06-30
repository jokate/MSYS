// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/StateTree/Task/YSAIStateTreeTaskInstanceData.h"

#include "StateTreeExecutionContext.h"
#include "AI/YSAIController.h"
#include "AI/Component/YSAIPerceptionComponent.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "General/YSStruct.h"

AYSAIController* UYSAIStateTreeTaskInstanceData::GetAIControllerFromContext(
	const FStateTreeExecutionContext& Context) const
{
	UObject* Object = Context.GetOwner();
	if ( IsValid(Object) == false )
	{
		return nullptr;
	}
	
	return Cast<AYSAIController>(Object);
}

EStateTreeRunStatus UYSAIStateTreeTask_TargetActor::EnterState(FStateTreeExecutionContext& Context,
                                                               const FStateTreeTransitionResult& Transition)
{
	AYSAIController* Controller = GetAIControllerFromContext(Context);
	
	if ( IsValid(Controller) == false )
	{
		return EStateTreeRunStatus::Failed;
	}
	
	UYSAIPerceptionComponent* AIPerceptionComponent = Controller->MainPerceptionComponent;
	
	if ( IsValid(AIPerceptionComponent) == false )
	{
		return EStateTreeRunStatus::Failed;
	}
	
	TargetingActorCollections = AIPerceptionComponent->GetTargetingActorCollections();
	
	if ( Policy == EYSTargetingPolicy::Fixed )
	{
		_SearchBestTarget(Context);
		return EStateTreeRunStatus::Succeeded;
	}
	
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus UYSAIStateTreeTask_TargetActor::Tick(FStateTreeExecutionContext& Context,
	const float DeltaTime)
{
	if ( Policy != EYSTargetingPolicy::Dynamic )
	{
		return EStateTreeRunStatus::Failed;
	} 
	
	CurrentSearchTime += DeltaTime;
	
	if ( CurrentSearchTime > SearchInterval )
	{
		_SearchBestTarget(Context);
		CurrentSearchTime = 0.f;
	}
	
	return Super::Tick(Context, DeltaTime);
}

void UYSAIStateTreeTask_TargetActor::ExitState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition)
{
	TargetingActorCollections = nullptr;
	
	Super::ExitState(Context, Transition);
}

void UYSAIStateTreeTask_TargetActor::_SearchBestTarget(const FStateTreeExecutionContext& Context) const
{
	AYSAIController* Controller = GetAIControllerFromContext(Context);

	if ( IsValid(Controller) == false || TargetingActorCollections.IsValid() == false )
	{
		return;
	}
	
	UEnvQueryManager* QueryManager = UEnvQueryManager::GetCurrent(Controller->GetWorld());

	if ( IsValid(QueryManager) == false )
	{
		return;
	} 
	
	FEnvQueryRequest Request(TargetingEQS, Controller);

	TSharedPtr<FEnvQueryResult> Result = QueryManager->RunInstantQuery(Request, EEnvQueryRunMode::Type::SingleResult);

	if ( Result.IsValid() == false )
	{
		if ( NeedToReleaseInvalidResult )
		{
			TargetingActorCollections->ResetBestTargetActor();
		}
		return;
	}
	
	TargetingActorCollections->SetBestTargetActor(Result->GetItemAsActor(0));
}
