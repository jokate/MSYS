// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/StateTree/Task/YSAIStateTreeTaskInstanceData.h"

#include "StateTreeExecutionContext.h"
#include "AI/YSAIController.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
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

	TargetingActorCollections = UYSAIPerceptionComponent::GetTargetingCollection(Controller);

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

	if ( Result.IsValid() == false || Result->IsAborted() || Result->Items.Num() == 0 )
	{
		if ( NeedToReleaseInvalidResult )
		{
			TargetingActorCollections->ResetBestTargetActor();
		}
		return;
	}
	
	AActor* BestTargetActor = Result->GetItemAsActor(0);
	TargetingActorCollections->SetBestTargetActor(BestTargetActor);
	// 주시(SetFocus)는 상태 수명에 묶인 별도 태스크(UYSAIStateTreeTask_FocusTarget)가 담당한다.
	// 검색 태스크에서 걸면 포커스를 풀 주체가 없어 모든 상태에서 계속 바라보게 된다.
}

AActor* UYSAIStateTreeTask_FocusTarget::GetFocusTarget(const FStateTreeExecutionContext& Context) const
{
	return UYSAIPerceptionComponent::GetBestTarget(GetAIControllerFromContext(Context));
}

EStateTreeRunStatus UYSAIStateTreeTask_FocusTarget::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition)
{
	AYSAIController* Controller = GetAIControllerFromContext(Context);
	if ( IsValid(Controller) == false )
	{
		return EStateTreeRunStatus::Failed;
	}

	if ( AActor* FocusTarget = GetFocusTarget(Context) )
	{
		Controller->SetFocus(FocusTarget);
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus UYSAIStateTreeTask_FocusTarget::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
	if ( bUpdateEveryTick == false )
	{
		return EStateTreeRunStatus::Running;
	}

	AYSAIController* Controller = GetAIControllerFromContext(Context);
	if ( IsValid(Controller) == false )
	{
		return EStateTreeRunStatus::Failed;
	}

	AActor* FocusTarget = GetFocusTarget(Context);
	if ( IsValid(FocusTarget) )
	{
		Controller->SetFocus(FocusTarget);
	}
	else
	{
		Controller->ClearFocus(EAIFocusPriority::Gameplay);
	}

	return EStateTreeRunStatus::Running;
}

void UYSAIStateTreeTask_FocusTarget::ExitState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition)
{
	if ( AYSAIController* Controller = GetAIControllerFromContext(Context) )
	{
		Controller->ClearFocus(EAIFocusPriority::Gameplay);
	}

	Super::ExitState(Context, Transition);
}

EStateTreeRunStatus UYSAIStateTreeTask_UseAbility::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition)
{
	AYSAIController* Controller = GetAIControllerFromContext(Context);
	UAbilitySystemComponent* ASC = IsValid(Controller) ? UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Controller->GetPawn()) : nullptr;

	if ( IsValid(ASC) == false || AbilityToUse.IsValid() == false )
	{
		return EStateTreeRunStatus::Failed;
	}

	// 발동 중에 바로 끝나는 어빌리티도 놓치지 않게 발동 전에 묶는다.
	bAbilityEnded = false;
	BoundASC = ASC;
	AbilityEndedHandle = ASC->OnAbilityEnded.AddUObject(this, &ThisClass::OnAbilityEnded);

	if ( ASC->TryActivateAbility(AbilityToUse) == false )
	{
		UnbindAbilityEnded();
		return EStateTreeRunStatus::Failed;
	}

	return bAbilityEnded ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Running;
}

EStateTreeRunStatus UYSAIStateTreeTask_UseAbility::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
	if ( bAbilityEnded )
	{
		return EStateTreeRunStatus::Succeeded;
	}

	return Super::Tick(Context, DeltaTime);
}

void UYSAIStateTreeTask_UseAbility::ExitState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition)
{
	// 상태를 나가도 어빌리티는 취소하지 않는다. 기다리기만 멈춘다.
	UnbindAbilityEnded();

	Super::ExitState(Context, Transition);
}

void UYSAIStateTreeTask_UseAbility::OnAbilityEnded(const FAbilityEndedData& AbilityEndedData)
{
	if ( AbilityEndedData.AbilitySpecHandle != AbilityToUse )
	{
		return;
	}

	bAbilityEnded = true;
	UnbindAbilityEnded();
}

void UYSAIStateTreeTask_UseAbility::UnbindAbilityEnded()
{
	if ( UAbilitySystemComponent* ASC = BoundASC.Get() )
	{
		ASC->OnAbilityEnded.Remove(AbilityEndedHandle);
	}

	AbilityEndedHandle.Reset();
	BoundASC.Reset();
}
