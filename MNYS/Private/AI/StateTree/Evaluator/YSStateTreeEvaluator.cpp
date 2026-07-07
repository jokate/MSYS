// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/StateTree/Evaluator/YSStateTreeEvaluator.h"

#include "StateTreeExecutionContext.h"
#include "AI/YSAIController.h"
#include "AI/Component/YSAIPerceptionComponent.h"
#include "General/YSStruct.h"
#include "StateTreeLinker.h"

bool FYSStateTreeEvaluatorTarget::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(AIControllerHandle);
	return true;
}

void FYSStateTreeEvaluatorTarget::TreeStart(FStateTreeExecutionContext& Context) const
{
	const AYSAIController* AIController = Context.GetExternalDataPtr(AIControllerHandle);
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	if (IsValid(AIController) == false || IsValid(AIController->MainPerceptionComponent) == false )
	{
		return;
	}
	
	UYSAIPerceptionComponent* MainPerception = AIController->MainPerceptionComponent;
	
	InstanceData.TargetingActorCollections = MainPerception->GetTargetingActorCollections();
}

void FYSStateTreeEvaluatorTarget::Tick(FStateTreeExecutionContext& Context, float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	if ( InstanceData.TargetingActorCollections == nullptr )
	{
		return;
	}
	
	const TSharedPtr<FYSTargetingActorCollections>& Targeting = InstanceData.TargetingActorCollections;

	if (Targeting.IsValid() == false)
	{
		InstanceData.BestTarget = nullptr;
		InstanceData.SkillTargets.Reset();
		return;
	}
	
	InstanceData.BestTarget = Targeting->GetBestTargetActor();
	
	TArray<AActor*> RawTargets = Targeting->GetSkillTargetActors();
	InstanceData.SkillTargets.Reset(RawTargets.Num());
	
	for (AActor* Target : RawTargets)
	{
		InstanceData.SkillTargets.Add(Target);
	}
}

void FYSStateTreeEvaluatorTarget::TreeStop(FStateTreeExecutionContext& Context) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	
	InstanceData.SkillTargets.Reset();
	InstanceData.BestTarget = nullptr;
	InstanceData.TargetingActorCollections = nullptr;
}
