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
	CapturedContext->ContextTags.Reset();
	CapturedContext->PendingEvaluatedIndex = INDEX_NONE;
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
	
	// 입력 컨텍스트 조건 즉시 판단.
	if ( bImmediateTransition )
	{
		// 실제 전환 시도 (bIsEvaluate=false로 실제 전환 수행)
		return DispatchNext(EYSPlaybackEvent::OnInput, false);
	}
	
	// 즉시적인 전환이 아닌 경우 평가 후 전환하는 방식으로..
	return DispatchNext(EYSPlaybackEvent::OnInput, true);  
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
	if (bIsEvaluate == false && CapturedContext->PendingEvaluatedIndex != INDEX_NONE)
	{
		const int32 ReservedIndex = CapturedContext->PendingEvaluatedIndex;
		CapturedContext->PendingEvaluatedIndex = INDEX_NONE;

		return CommitTransition(ReservedIndex);
	}

	const FYSPlaybackEdge* MatchedEdge = FindTransitionEdge(Event);

	// 만약 맞는 게 없다고 해도 그냥 넘어갈 수 있어야 함. 그렇게 되면 정확한 인풋만을 요구하게 되기 떄문임.
	// 단, 그렇게 된다고 하니까, 없네...
	if (MatchedEdge == nullptr)
	{
		// 평가는 조건 조회일 뿐이므로 체인에 손대지 않는다.
		return bIsEvaluate ? false : HandleUnmatchedEvent(Event);
	}
	
	ProcessConditionMatch(*MatchedEdge, CapturedContext);
	
	if (bIsEvaluate)
	{
		// 평가 모드는 전환하지 않고 예약만 남긴다.
		// 체인 종료(-1)는 예약 대상이 아니다 — 예약해두면 종료가 엉뚱한 이벤트에 딸려 나간다.
		if (Ability->GetPlaybackNode(MatchedEdge->NextNodeIndex) == nullptr)
		{
			return false;
		}

		CapturedContext->PendingEvaluatedIndex = MatchedEdge->NextNodeIndex;
		return true;
	}

	return CommitTransition(MatchedEdge->NextNodeIndex);
}

const FYSPlaybackEdge* UYSAbilityPlaybackBase::FindTransitionEdge(EYSPlaybackEvent Event) const
{
	for (const FYSPlaybackEdge& Edge : Transitions)
	{
		if (Edge.RequiredResult != Event)
		{
			continue;
		}

		if (AreConditionsSatisfied(Edge))
		{
			return &Edge;
		}
	}

	return nullptr;
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
