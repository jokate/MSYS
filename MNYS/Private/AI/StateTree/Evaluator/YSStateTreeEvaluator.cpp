// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/StateTree/Evaluator/YSStateTreeEvaluator.h"

#include "StateTreeExecutionContext.h"
#include "AI/YSAIController.h"
#include "AI/Component/YSAIPerceptionComponent.h"
#include "General/YSStruct.h"
#include "YSAbilitySystemComponent.h"
#include "Ability/AIAbility/YSGameplayAbility_AIBase.h"

void FYSStateTreeEvaluatorTarget::Tick(FStateTreeExecutionContext& Context, float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	InstanceData.BestTarget = nullptr;
	InstanceData.SkillTargets.Reset();

	const TSharedPtr<FYSTargetingActorCollections> Targeting = UYSAIPerceptionComponent::GetTargetingCollection(Cast<AActor>(Context.GetOwner()));

	if (Targeting.IsValid() == false)
	{
		return;
	}
	
	InstanceData.BestTarget = Targeting->GetBestTargetActor();
	
	for (AActor* Target : Targeting->GetSkillTargetActors())
	{
		InstanceData.SkillTargets.Add(Target);
	}
}

void FYSStateTreeEvaluatorTarget::TreeStop(FStateTreeExecutionContext& Context) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	
	InstanceData.SkillTargets.Reset();
	InstanceData.BestTarget = nullptr;
}

void FYSStateTreeEvaluatorAbilityActivation::Tick(FStateTreeExecutionContext& Context, float DeltaTime) const
{
	// 폰 · ASC 를 못 찾은 틱에도 앞 틱 결과가 남지 않게 먼저 비운다.
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	InstanceData = FInstanceDataType();

	const AYSAIController* AIController = Cast<AYSAIController>(Context.GetOwner());
	UYSAbilitySystemComponent* YSASC = UYSAbilitySystemComponent::Get(IsValid(AIController) ? AIController->GetPawn() : nullptr);

	const FYSAIAbilitySelection Selection = UYSGameplayAbility_AIBase::SelectAbility(YSASC);
	const FYSAIAbilityScore* Best = Selection.GetBest();

	if ( Best == nullptr )
	{
		return;
	}

	InstanceData.bIsEnableActivateAbility = true;
	InstanceData.BestAbilitySpecHandle = Best->Handle;
	InstanceData.EnvQueryInstance = Best->Ability->GetQueryToReposition();
}
