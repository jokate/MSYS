// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/AbilityComponent/YSAbilityPlayback.h"

#include "AbilitySystemBlueprintLibrary.h"

#include "Ability/YSGameplayAbility.h"
#include "YSAbilitySystemComponent.h"
#include "Ability/AbilityComponent/YSPlaybackAction.h"
#include "Ability/AbilityComponent/YSPlaybackCondition.h"
#include "Ability/AbilityComponent/YSPlaybackTask.h"

void UYSAbilityPlaybackBase::Play(const TSharedPtr<FYSPlaybackContext>& Context)
{
	CapturedContext = Context;
	const int32 Serial = ++PlaySerial;
	
	
	for ( const UYSPlaybackAction* Action : EnterActions )
	{
		if ( IsValid(Action) )
		{
			Action->Execute(CapturedContext);
		}
	}
	
	if ( Tasks.Num() == 0 )
	{
		RequestTransition(INDEX_NONE);
		return;
	}

	for ( UYSPlaybackTask* Task : Tasks )
	{
		if ( IsValid(Task) == false )
		{
			continue;
		}

		Task->Start(this);

		// 시작하자마자 전환이 확정됐다. 이 노드는 이미 끝났거나 새로 시작됐다.
		if ( PlaySerial != Serial )
		{
			return;
		}
	}
}

void UYSAbilityPlaybackBase::ReleaseMotionWarp()
{
	// 모션 워프는 몽타주 재생 Task 가 걸었다. 건 쪽이 푼다.
	for ( UYSPlaybackTask* Task : Tasks )
	{
		if ( IsValid(Task) )
		{
			Task->ReleaseMotionWarp();
		}
	}
}

bool UYSAbilityPlaybackBase::HandleInput(const FGameplayTag& InputTag, EYSInputPhase InputPhase)
{
	if ( CapturedContext.IsValid() == false )
	{
		return false;
	}

	for ( UYSPlaybackTask* Task : Tasks )
	{
		if ( IsValid(Task) && Task->IsRunning() && Task->HandleInput(InputTag, InputPhase) )
		{
			return true;
		}
	}

	if ( bEndChainOnUnmatchedInput )
	{
		RequestTransition(INDEX_NONE);
	}

	return false;
}

void UYSAbilityPlaybackBase::HandleInputWindowClosed()
{
	if ( CapturedContext.IsValid() == false )
	{
		return;
	}

	for ( UYSPlaybackTask* Task : Tasks )
	{
		if ( IsValid(Task) && Task->IsRunning() && Task->HandleInputWindowClosed() )
		{
			return;
		}
	}

	if ( bEndChainOnUnmatchedInput )
	{
		RequestTransition(INDEX_NONE);
	}
}

void UYSAbilityPlaybackBase::EndPlay()
{
	++PlaySerial;

	for ( UYSPlaybackTask* Task : Tasks )
	{
		if ( IsValid(Task) )
		{
			Task->Stop();
		}
	}

	CapturedContext = nullptr;
}

void UYSAbilityPlaybackBase::OnHit(const TArray<FHitResult>& HitResults)
{	
	const int32 Serial = PlaySerial;

	for ( UYSPlaybackTask* Task : Tasks )
	{
		if ( IsValid(Task) && Task->IsRunning() )
		{
			Task->HandleHit(HitResults);
		}

		if ( PlaySerial != Serial )
		{
			return;
		}
	}
}

void UYSAbilityPlaybackBase::HandleContextTagChanged()
{
	const int32 Serial = PlaySerial;

	for ( UYSPlaybackTask* Task : Tasks )
	{
		if ( IsValid(Task) && Task->IsRunning() )
		{
			Task->HandleContextTagChanged();
		}

		if ( PlaySerial != Serial )
		{
			return;
		}
	}
}

bool UYSAbilityPlaybackBase::RequestTransition(int32 NextNodeIndex)
{
	UYSGameplayAbility* Ability = GetCurrentPlaybackOwningAbility();

	if ( IsValid(Ability) == false )
	{
		return false;
	}

	// ActivePlayback 이 이 노드의 EndPlay(모든 EndTask)를 다음 노드의 Play 보다 먼저 부른다.
	if ( NextNodeIndex == INDEX_NONE )
	{
		Ability->NotifyPlaybackChainFinished();
	}
	else
	{
		Ability->ActivePlayback(NextNodeIndex);
	}

	return true;
}

bool UYSAbilityPlaybackBase::TryResolveRoutes(const TArray<FYSPlaybackRoute>& Routes)
{
	if ( CapturedContext.IsValid() == false )
	{
		return false;
	}

	for ( const FYSPlaybackRoute& Route : Routes )
	{
		if ( AreConditionsSatisfied(Route.Conditions) == false )
		{
			continue;
		}

		// 전환이 확정된 시점에만 부른다 — 여기서 입력이 소비된다.
		ConsumeConditions(Route.Conditions);

		// 행동은 전환보다 먼저 돈다. 체인이 끝나면 어빌리티의 이벤트 대기 Task 가 함께 죽어 이벤트를 받을 곳이 없다.
		const int32 Serial = PlaySerial;

		for ( const UYSPlaybackAction* Action : Route.Actions )
		{
			if ( IsValid(Action) )
			{
				Action->Execute(CapturedContext);
			}
		}

		// 행동이 일으킨 이벤트가 이미 이 노드를 끝냈다.
		if ( PlaySerial != Serial )
		{
			return true;
		}

		switch ( Route.Target )
		{
		case EYSRouteTarget::Stay :
			return true;
		case EYSRouteTarget::Node :
			return RequestTransition(Route.NextNodeIndex);
		default :
			return RequestTransition(INDEX_NONE);
		}
	}

	return false;
}

bool UYSAbilityPlaybackBase::AreConditionsSatisfied(const TArray<FInstancedStruct>& Conditions) const
{
	// 조건은 AND 결합이다. 하나라도 실패하면 나머지는 볼 필요가 없다.
	for ( const FInstancedStruct& InstancedStruct : Conditions )
	{
		const FYSPlaybackCondition* Condition = InstancedStruct.GetPtr<FYSPlaybackCondition>();

		if ( Condition == nullptr )
		{
			continue;
		}

		if ( Condition->Evaluate(CapturedContext) == false )
		{
			return false;
		}
	}

	return true;
}

void UYSAbilityPlaybackBase::ConsumeConditions(const TArray<FInstancedStruct>& Conditions) const
{
	for ( const FInstancedStruct& InstancedStruct : Conditions )
	{
		if ( const FYSPlaybackCondition* Condition = InstancedStruct.GetPtr<FYSPlaybackCondition>() )
		{
			Condition->OnConditionEvaluatedComplete(CapturedContext);
		}
	}
}
