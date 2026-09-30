// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/AbilityComponent/YSPlaybackTask.h"

#include "Abilities/Tasks/AbilityTask.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Ability/YSGameplayAbility.h"
#include "Ability/AbilityComponent/YSAbilityPlayback.h"
#include "Ability/MontageSelector/YSMontageSelector.h"
#include "Ability/Task/YSAT_PlaySequence.h"

#define LOCTEXT_NAMESPACE "YSPlaybackTask"

void UYSPlaybackTask::Start(UYSAbilityPlaybackBase* InOwnerNode)
{
	OwnerNode = InOwnerNode;
	RunningTasks.Reset();
	bRunning = true;
	OnStart();
}

void UYSPlaybackTask::Stop()
{
	if ( bRunning == false )
	{
		return;
	}

	// 먼저 내려야 EndTask 도중 되돌아오는 콜백이 전환을 다시 요청하지 못한다.
	bRunning = false;

	for ( UAbilityTask* Task : RunningTasks )
	{
		// 스스로 끝난 Task 는 이미 가비지 표시라 IsValid 에서 걸러진다.
		if ( IsValid(Task) )
		{
			Task->EndTask();
		}
	}

	RunningTasks.Reset();
	OnStop();
}

bool UYSPlaybackTask::TryResolve(int32 OutputIndex)
{
	if ( bRunning == false || IsValid(OwnerNode) == false )
	{
		return false;
	}

	static const TArray<FYSPlaybackRoute> NoRoutes;
	const TArray<FYSPlaybackRoute>& Routes = OutputRoutes.IsValidIndex(OutputIndex) ? OutputRoutes[OutputIndex].Routes : NoRoutes;

	return OwnerNode->TryResolveRoutes(Routes);
}

void UYSPlaybackTask::Resolve(int32 OutputIndex)
{
	if ( bRunning == false || IsValid(OwnerNode) == false )
	{
		return;
	}

	// 경로가 전부 실패하면 체인을 끝낸다. 예전 HandleUnmatchedEvent 와 같은 결과다.
	if ( TryResolve(OutputIndex) == false )
	{
		OwnerNode->RequestTransition(INDEX_NONE);
	}
}

FText UYSPlaybackTask::GetOutputDisplayName(int32 OutputIndex) const
{
	return LOCTEXT("OutputCompleted", "완료");
}

void UYSPlaybackTask::ResetRoutes()
{
	OutputRoutes.Reset();
	OutputRoutes.SetNum(GetOutputCount());
}

void UYSPlaybackTask::AddRoute(int32 OutputIndex, const FYSPlaybackRoute& Route)
{
	if ( OutputIndex < 0 )
	{
		return;
	}

	if ( OutputRoutes.Num() <= OutputIndex )
	{
		OutputRoutes.SetNum(OutputIndex + 1);
	}

	OutputRoutes[OutputIndex].Routes.Add(Route);
}

UYSGameplayAbility* UYSPlaybackTask::GetOwningAbility() const
{
	return IsValid(OwnerNode) ? OwnerNode->GetCurrentPlaybackOwningAbility() : nullptr;
}

TSharedPtr<FYSPlaybackContext> UYSPlaybackTask::GetContext() const
{
	return IsValid(OwnerNode) ? OwnerNode->GetContext() : nullptr;
}

void UYSPlaybackTask_PlayMontage::ReleaseMotionWarp()
{
	const FYSMontageSelector* Selector = MontageSelector.GetPtr<FYSMontageSelector>();
	UYSGameplayAbility* Ability = GetOwningAbility();

	if ( Selector == nullptr || IsValid(Ability) == false )
	{
		return;
	}

	Selector->SetMotionWarp(Ability, false);
}

void UYSPlaybackTask_PlayMontage::OnStart()
{
	const FYSMontageSelector* Selector = MontageSelector.GetPtr<FYSMontageSelector>();
	UYSGameplayAbility* Ability = GetOwningAbility();

	if ( Selector == nullptr || IsValid(Ability) == false )
	{
		return;
	}

	UAnimMontage* Montage = Selector->SelectMontage(Ability);

	if ( IsValid(Montage) )
	{
		UAbilityTask_PlayMontageAndWait* Task = Track(UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(Ability, TEXT("PlayMontage"), Montage));

		if ( IsValid(Task) )
		{
			Task->OnCompleted.AddDynamic(this, &ThisClass::OnMontageFinished);
			Task->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageFinished);
			Task->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageInterrupted);
			Task->OnCancelled.AddDynamic(this, &ThisClass::OnMontageInterrupted);
			Task->ReadyForActivation();
		}
	}

	Selector->SetMotionWarp(Ability, true);
}

FText UYSPlaybackTask_PlayMontage::GetOutputDisplayName(int32 OutputIndex) const
{
	return ( OutputIndex == Output_Interrupted )
		? LOCTEXT("OutputInterrupted", "중단")
		: LOCTEXT("OutputCompleted", "완료");
}

void UYSPlaybackTask_PlayMontage::OnMontageFinished()
{
	Resolve(Output_Completed);
}

void UYSPlaybackTask_PlayMontage::OnMontageInterrupted()
{
	Resolve(Output_Interrupted);
}

void UYSPlaybackTask_PlaySequence::OnStart()
{
	UYSGameplayAbility* Ability = GetOwningAbility();
	const TSharedPtr<FYSPlaybackContext> Context = GetContext();

	if ( IsValid(Ability) == false || Context.IsValid() == false )
	{
		return;
	}

	UYSAT_PlaySequence* Task = Track(UYSAT_PlaySequence::CreatePlaySequenceTask(Ability, SequenceSettings, Context->Instigator, Context->Target));

	if ( IsValid(Task) )
	{
		Task->OnSequenceFinishedDelegate.AddDynamic(this, &ThisClass::OnSequenceFinished);
		Task->ReadyForActivation();
	}
}

void UYSPlaybackTask_PlaySequence::OnSequenceFinished()
{
	Resolve();
}

void UYSPlaybackTask_WaitInput::OnStart()
{
	bInputReceived = false;
}

void UYSPlaybackTask_WaitEvent::OnStart()
{
	UAbilityTask_WaitGameplayEvent* Task = Track(UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(GetOwningAbility(), EventTag, nullptr, false));
	
	if ( IsValid(Task) )
	{
		Task->EventReceived.AddDynamic(this, &ThisClass::OnEventReceived);
		Task->ReadyForActivation();
	}
}

FText UYSPlaybackTask_WaitEvent::GetOutputDisplayName(int32 OutputIndex) const
{
	return LOCTEXT("OutputEvent", "이벤트");
}

void UYSPlaybackTask_WaitEvent::OnEventReceived(FGameplayEventData Payload)
{
	Resolve();
}

FText UYSPlaybackTask_WaitInput::GetOutputDisplayName(int32 OutputIndex) const
{
	return LOCTEXT("OutputInput", "입력");
}

bool UYSPlaybackTask_WaitInput::HandleInput(const FGameplayTag& InInputTag, EYSInputPhase InInputPhase)
{
	if ( InputTag.IsValid() == false || InInputTag != InputTag || InInputPhase != InputPhase )
	{
		return false;
	}

	if ( bWaitInputWindowClose )
	{
		bInputReceived = true;
		return true;
	}

	// 경로가 전부 실패하면 false — 노드가 다음 Task 에 기회를 준다.
	return TryResolve();
}

bool UYSPlaybackTask_WaitInput::HandleInputWindowClosed()
{
	if ( bWaitInputWindowClose == false )
	{
		return false;
	}

	if ( InputTag.IsValid() && bInputReceived == false )
	{
		return false;
	}

	// 유지 경로로 끝나면 이 Task 는 계속 대기한다. 다음 창은 새 입력을 받아야 한다.
	bInputReceived = false;

	return TryResolve();
}

void UYSPlaybackTask_WaitDelay::OnStart()
{
	UYSGameplayAbility* Ability = GetOwningAbility();

	if ( IsValid(Ability) == false )
	{
		return;
	}

	UAbilityTask_WaitDelay* Task = Track(UAbilityTask_WaitDelay::WaitDelay(Ability, DelayTime));

	if ( IsValid(Task) )
	{
		Task->OnFinish.AddDynamic(this, &ThisClass::OnDelayFinished);
		Task->ReadyForActivation();
	}
}

void UYSPlaybackTask_WaitDelay::OnDelayFinished()
{
	Resolve();
}

#undef LOCTEXT_NAMESPACE
