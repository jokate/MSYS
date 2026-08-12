// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/AbilityComponent/YSAbilityPlayback.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "DefaultLevelSequenceInstanceData.h"
#include "LevelSequence.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "MovieSceneSequencePlaybackSettings.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Ability/YSGameplayAbility.h"
#include "Ability/MontageSelector/YSMontageSelector.h"
#include "Ability/Payload/YSAbilityTriggerPayload.h"
#include "Ability/AbilityComponent/YSPlaybackCondition.h"
#include "Character/Components/YSCameraManageComponent.h"
#include "General/YSGameplayTag.h"

void UYSAbilityPlaybackBase::SetPlayback(TSharedPtr<FYSPlaybackContext> Context)
{
	CapturedContext = Context;
	ProcessContextBeforePlay();
	
	switch ( PlaybackType )
	{
	case EYSAbilityPlaybackType::Montage :
		SetupMontage();
		break;
	case EYSAbilityPlaybackType::Sequence :
		SetupSequence();
		break;
	default :
		break;
	}
	
	// 기존에 받은 정보들은 그냥 없앰 처리.
	// 예약은 노드보다 오래 사는 컨텍스트에 있으므로, 물려받으면 엉뚱한 엣지로 점프한다.
	CapturedContext->ContextTags.Reset();
	CapturedContext->PendingEvaluatedEdgeIndex = INDEX_NONE;
}

void UYSAbilityPlaybackBase::OnSequencePlayed()
{
	DispatchNext(EYSPlaybackEvent::Completed);
}

void UYSAbilityPlaybackBase::Play(const TSharedPtr<FYSPlaybackContext>& Context)
{
	SetPlayback(Context);
}

void UYSAbilityPlaybackBase::ReleaseMotionWarp()
{
	const FYSMontageSelector* CurMontageSelector = MontageSelector.GetPtr<FYSMontageSelector>();

	if ( CurMontageSelector == nullptr )
	{
		return;
	};

	CurMontageSelector->SetMotionWarp(CapturedContext->OwnerAbility, false);
}

void UYSAbilityPlaybackBase::EndPlay()
{
	if ( IsValid(PlayMontageAndWaitTask) )
	{
		PlayMontageAndWaitTask->EndTask();
	}
	
	if ( IsValid(LevelSequencePlayer) )
	{
		LevelSequencePlayer->OnFinished.RemoveAll(this);
		LevelSequencePlayer->Stop();
	}
	
	CapturedContext = nullptr;
}

bool UYSAbilityPlaybackBase::TryAcceptInputTag()
{
	// Play() 이전에 입력이 들어올 수 있다. 컨텍스트가 없으면 소비하지 않고 흘려보낸다.
	if ( CapturedContext.IsValid() == false )
	{
		return false;
	}
	
	// 예약 소진 경로(DispatchNext 선두)를 타지 않고 직접 엣지를 찾는다.
	// 예약이 하나 걸려 있다고 해서 뒤이어 온 입력을 못 본 척하면,
	// 발사 중 버튼을 떼도 몽타주가 끝날 때까지 반응하지 못한다.
	const int32 MatchedEdgeIndex = FindTransitionEdgeIndex(EYSPlaybackEvent::OnInput);

	if ( Transitions.IsValidIndex(MatchedEdgeIndex) == false )
	{
		return HandleUnmatchedEvent(EYSPlaybackEvent::OnInput);
	}

	const FYSPlaybackEdge& MatchedEdge = Transitions[MatchedEdgeIndex];

	// 즉시 엣지는 재생 중인 몽타주를 끊고 지금 전환한다 (조준 취소, 차지 릴리즈).
	// 이벤트만 쏘는 엣지도 여기로 온다 — 몽타주를 건드리지 않으니 미룰 이유가 없고,
	// 오히려 미루면 "조준 중 클릭했는데 조준이 끝나야 분신이 나오는" 꼴이 된다.
	if ( MatchedEdge.bImmediateTransition || MatchedEdge.bFireEventOnly )
	{
		return CommitEdge(MatchedEdge);
	}

	// 예약 엣지는 몽타주를 존중하고 완료 시점에 전환한다 (콤보, 연사 루프).
	return ReserveEdge(MatchedEdgeIndex);
}

void UYSAbilityPlaybackBase::OnMontagePlayed()
{
	DispatchNext(EYSPlaybackEvent::Completed);
}

void UYSAbilityPlaybackBase::OnMontageInterrupted()
{
	DispatchNext(EYSPlaybackEvent::Interrupted);
}

void UYSAbilityPlaybackBase::SetupMontage()
{
	const FYSMontageSelector* CurMontageSelector = MontageSelector.GetPtr<FYSMontageSelector>();

	if ( CurMontageSelector == nullptr )
	{
		return;
	};

	UYSGameplayAbility* OwningAbility = GetCurrentPlaybackOwningAbility();
	
	if ( IsValid(OwningAbility) == false )
	{
		return;
	}
 	
	UAnimMontage* TargetToPlayMontage = CurMontageSelector->SelectMontage(OwningAbility);
	
	if ( IsValid(TargetToPlayMontage) ) 
	{
		PlayMontageAndWaitTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(OwningAbility, 
			TEXT("PlayMontage"), TargetToPlayMontage);
		if ( IsValid(PlayMontageAndWaitTask))
		{
			PlayMontageAndWaitTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontagePlayed);
			PlayMontageAndWaitTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageInterrupted);
			PlayMontageAndWaitTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontagePlayed);
			PlayMontageAndWaitTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageInterrupted);
			PlayMontageAndWaitTask->ReadyForActivation();	
		}
	}

	CurMontageSelector->SetMotionWarp(OwningAbility, true);
}

void UYSAbilityPlaybackBase::SetupSequence()
{		
	// Sequencer가 SkeletalMesh 애니메이션의 완전한 제어권을 갖도록,
	// 진입 전에 AnimInstance 슬롯을 점유 중인 선행 몽타주를 명시적으로 정지한다.
	// CancelAbilitiesWithTags 대신 CurrentMontageStop을 사용하는 이유:
	// 어빌리티 라이프사이클을 건드리지 않고 슬롯 레이어만 클리어하기 위함.
	AActor* Instigator = GetCurrentPlaybackInstigator();
	
	if ( IsValid(Instigator) == false )
	{
		return;
	}
	
	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Instigator))
	{
		ASC->CurrentMontageStop(SequenceSettings.MontageBlendOutTime);
	}

	ULevelSequence* Sequence = SequenceSettings.Sequence.LoadSynchronous();
	if (!IsValid(Sequence))
	{
		return;
	}
	
	FMovieSceneSequencePlaybackSettings Settings;
	Settings.PlayRate = SequenceSettings.PlayRate;
	Settings.bPauseAtEnd = false;
	ALevelSequenceActor* SequenceActor = nullptr;
	LevelSequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer( this, Sequence , Settings, SequenceActor);
	
	if (!IsValid(LevelSequencePlayer) || !IsValid(SequenceActor))
	{
		return;
	}
	
	ActiveSequenceActor = SequenceActor;

	ActiveSequenceActor->SetBindingByTag(TEXT("Player"), { Instigator });
	
	if (ActiveSequenceActor->FindNamedBinding(TEXT("Enemy")).IsValid())	
	{
		AActor* CapturedTarget = GetCurrentPlaybackTarget();
		if ( IsValid(CapturedTarget) )
		{
			UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(CapturedTarget);
			
			if (IsValid(TargetASC) )
			{
				TargetASC->CurrentMontageStop(SequenceSettings.MontageBlendOutTime);
			}
			
        	ActiveSequenceActor->SetBindingByTag(TEXT("Enemy"), { CapturedTarget });	
		}
	}
	
	if (!SequenceSettings.bOverrideCameraBySequence && IsValid(ActiveSequenceActor))
	{
		// 카메라 컷 트랙 비활성화
		ActiveSequenceActor->bOverrideInstanceData = false;
	}
	
	LevelSequencePlayer->OnFinished.AddDynamic(this, &ThisClass::OnSequencePlayed);
	LevelSequencePlayer->Play();
}

bool UYSAbilityPlaybackBase::DispatchNext(EYSPlaybackEvent Event, bool bIsEvaluate)
{
	UYSGameplayAbility* Ability = GetCurrentPlaybackOwningAbility();

	if (IsValid(Ability) == false || CapturedContext.IsValid() == false)
	{
		return true;
	}

	// 실행 모드로 들어왔고 예약이 있으면 그것부터 소진한다.
	// 평가 단계에서 이미 조건을 통과시킨 전환이므로 재평가하지 않는다.
	if (bIsEvaluate == false && Transitions.IsValidIndex(CapturedContext->PendingEvaluatedEdgeIndex))
	{
		const FYSPlaybackEdge& ReservedEdge = Transitions[CapturedContext->PendingEvaluatedEdgeIndex];
		CapturedContext->PendingEvaluatedEdgeIndex = INDEX_NONE;

		return CommitEdge(ReservedEdge);
	}

	const int32 MatchedEdgeIndex = FindTransitionEdgeIndex(Event);
	
	if (Transitions.IsValidIndex(MatchedEdgeIndex) == false)
	{
		// 평가는 조건 조회일 뿐이므로 체인에 손대지 않는다.
		return bIsEvaluate ? false : HandleUnmatchedEvent(Event);
	}
	
	if (bIsEvaluate)
	{
		return ReserveEdge(MatchedEdgeIndex);
	}

	return CommitEdge(Transitions[MatchedEdgeIndex]);
}

int32 UYSAbilityPlaybackBase::FindTransitionEdgeIndex(EYSPlaybackEvent Event) const
{
	for (int32 EdgeIndex = 0; EdgeIndex < Transitions.Num(); ++EdgeIndex)
	{
		const FYSPlaybackEdge& Edge = Transitions[EdgeIndex];

		if (Edge.RequiredResult != Event)
		{
			continue;
		}

		if (AreConditionsSatisfied(Edge))
		{
			return EdgeIndex;
		}
	}

	return INDEX_NONE;
}

bool UYSAbilityPlaybackBase::CommitEdge(const FYSPlaybackEdge& Edge)
{
	ProcessConditionMatch(Edge, CapturedContext);
	
	if ( Edge.TriggerGameplayData.IsValid())
	{
		FireEdgeEvent(Edge.TriggerGameplayData);
	}
	
	if ( Edge.bFireEventOnly )
	{
		return true;
	}

	return CommitTransition(Edge.NextNodeIndex);
}

void UYSAbilityPlaybackBase::FireEdgeEvent(const FGameplayEventSendData& SendEventData) const 
{
	UYSGameplayAbility* Ability = GetCurrentPlaybackOwningAbility();

	if ( IsValid(Ability) == false || CapturedContext.IsValid() == false )
	{
		return;
	}

	UAbilitySystemComponent* ASC = Ability->GetAbilitySystemComponentFromActorInfo();

	if ( IsValid(ASC) == false )
	{
		return;
	}

	FGameplayEventData EventData;
	EventData.EventTag = SendEventData.TargetToTrigger;

	const TArray<UYSAbilityTriggerPayload*>& Payloads = SendEventData.TriggerPayloads;
	for ( int32 i = 0; i < 2; ++i )
	{
		if ( Payloads.IsValidIndex(i) == false )
		{
			continue;
		}

		i == 0 ? EventData.OptionalObject = Payloads[i] : EventData.OptionalObject2 = Payloads[i];
	}
	
	EventData.Instigator = CapturedContext->Instigator;
	EventData.Target = CapturedContext->Target;

	ASC->HandleGameplayEvent(SendEventData.TargetToTrigger, &EventData);
}

bool UYSAbilityPlaybackBase::ReserveEdge(int32 EdgeIndex)
{
	if (Transitions.IsValidIndex(EdgeIndex) == false || CapturedContext.IsValid() == false)
	{
		return false;
	}

	UYSGameplayAbility* Ability = GetCurrentPlaybackOwningAbility();

	// 체인 종료(-1)는 예약 대상이 아니다 — 예약해두면 종료가 엉뚱한 이벤트에 딸려 나간다.
	if (IsValid(Ability) == false || Ability->GetPlaybackNode(Transitions[EdgeIndex].NextNodeIndex) == nullptr)
	{
		return false;
	}

	CapturedContext->PendingEvaluatedEdgeIndex = EdgeIndex;
	return true;
}

bool UYSAbilityPlaybackBase::AreConditionsSatisfied(const FYSPlaybackEdge& Edge) const
{
	// 조건은 AND 결합이다. 하나라도 실패하면 나머지는 볼 필요가 없다.
	for (const FInstancedStruct& InstancedStruct : Edge.TransitionConditions)
	{
		const FYSPlaybackCondition* Condition = InstancedStruct.GetPtr<FYSPlaybackCondition>();

		if (Condition == nullptr)
		{
			continue;
		}

		if (Condition->Evaluate(CapturedContext) == false)
		{
			return false;
		}
	}

	return true;
}

bool UYSAbilityPlaybackBase::CommitTransition(int32 NextNodeIndex)
{
	UYSGameplayAbility* Ability = GetCurrentPlaybackOwningAbility();

	if (IsValid(Ability) == false)
	{
		return false;
	}

	if (NextNodeIndex == INDEX_NONE)
	{
		Ability->NotifyPlaybackChainFinished();
	}
	else
	{
		Ability->ActivePlayback(NextNodeIndex);
	}

	return true;
}

bool UYSAbilityPlaybackBase::HandleUnmatchedEvent(EYSPlaybackEvent Event)
{
	// 입력이 어느 엣지와도 안 맞은 경우에만 이 플래그를 본다.
	// 몽타주 완료 등 다른 이벤트까지 막으면 대기 노드에서 어빌리티가 영영 매달린다.
	if (Event == EYSPlaybackEvent::OnInput && bEndChainOnUnmatchedInput == false)
	{
		return false;
	}

	CommitTransition(INDEX_NONE);
	return false;
}

void UYSAbilityPlaybackBase::ProcessConditionMatch(const FYSPlaybackEdge& Edge,
	const TSharedPtr<FYSPlaybackContext>& Context) const
{
	for (const FInstancedStruct& InstancedStruct : Edge.TransitionConditions)
	{
		const FYSPlaybackCondition* Condition = InstancedStruct.GetPtr<FYSPlaybackCondition>();

		if (Condition == nullptr)
		{
			continue;
		}

		Condition->OnConditionEvaluatedComplete(Context);
	}
}

void UYSAbilityPlayback_FirstHitTarget::ProcessContextBeforePlay()
{
	UYSGameplayAbility* OwnerAbility = CapturedContext->OwnerAbility;
	 
	if ( IsValid(OwnerAbility) == false )
	{
		return;
	}
	
	TSharedPtr<FYSAbilityHitContext> HitContext = OwnerAbility->GetHitContext();

	if ( HitContext == nullptr )
	{
		return;
	}
	
	const TArray<AActor*>& HitActors = HitContext.Get()->GetAllHitActors();
	
	if ( HitActors.Num() == 0 )
	{
		return;
	}
	
	CapturedContext->Target = HitActors[0];
}

void UYSAbilityPlayback_ReleaseBuff::ProcessContextBeforePlay()
{
	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(CapturedContext->Instigator))
	{
		ASC->RemoveActiveEffectsWithTags(BuffTags);
	}
}

void UYSAbilityPlaybackBase::OnHit(const TArray<FHitResult>& HitResults)
{	
	DispatchNext(EYSPlaybackEvent::OnHitTarget, true);
}
