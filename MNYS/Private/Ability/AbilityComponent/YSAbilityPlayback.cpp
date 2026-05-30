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
#include "Character/Components/YSLockOnComponent.h"

void UYSAbilityPlaybackBase::SetPlayback(const FYSPlaybackContext& Context)
{
	CapturedContext = Context;
	
	ProcessContextBeforePlay();
	
	switch ( PlaybackType )
	{
	case EYSAbilityPlaybackType::Montage :
		SetupMontage(Context);
		break;
	case EYSAbilityPlaybackType::Sequence :
		SetupSequence(Context);
		break;
	default :
		break;
	}
	
	CapturedContext.HitResults.Reset(); 
}

void UYSAbilityPlaybackBase::OnSequencePlayed()
{
	DispatchNext(EYSPlaybackResult::Completed);
}

void UYSAbilityPlaybackBase::Play(const FYSPlaybackContext& Context)
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

	CurMontageSelector->SetMotionWarp(CapturedContext.OwnerAbility, false);
}

bool UYSAbilityPlaybackBase::CheckCondition(const FYSPlaybackContext& Context)
{
	return true;
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
}

void UYSAbilityPlaybackBase::OnMontagePlayed()
{
	DispatchNext(EYSPlaybackResult::Completed);
}

void UYSAbilityPlaybackBase::OnMontageInterrupted()
{
	DispatchNext(EYSPlaybackResult::Interrupted);
}

void UYSAbilityPlaybackBase::SetupMontage(const FYSPlaybackContext& Context)
{
	const FYSMontageSelector* CurMontageSelector = MontageSelector.GetPtr<FYSMontageSelector>();

	if ( CurMontageSelector == nullptr )
	{
		return;
	};

	UAnimMontage* TargetToPlayMontage = CurMontageSelector->SelectMontage(CapturedContext.OwnerAbility);
	
	if ( IsValid(TargetToPlayMontage) ) 
	{
		PlayMontageAndWaitTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(CapturedContext.OwnerAbility, 
			TEXT("PlayMontage"), TargetToPlayMontage);
		if ( IsValid(PlayMontageAndWaitTask))
		{
			PlayMontageAndWaitTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontagePlayed);
			PlayMontageAndWaitTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageInterrupted);
			PlayMontageAndWaitTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageInterrupted);
			PlayMontageAndWaitTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageInterrupted);
			PlayMontageAndWaitTask->ReadyForActivation();	
		}
	}

	CurMontageSelector->SetMotionWarp(CapturedContext.OwnerAbility, true);
}

void UYSAbilityPlaybackBase::SetupSequence(const FYSPlaybackContext& Context)
{		
	// Sequencer가 SkeletalMesh 애니메이션의 완전한 제어권을 갖도록,
	// 진입 전에 AnimInstance 슬롯을 점유 중인 선행 몽타주를 명시적으로 정지한다.
	// CancelAbilitiesWithTags 대신 CurrentMontageStop을 사용하는 이유:
	// 어빌리티 라이프사이클을 건드리지 않고 슬롯 레이어만 클리어하기 위함.
	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Context.Instigator))
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

	ActiveSequenceActor->SetBindingByTag(TEXT("Player"), { Context.Instigator });
	
	if (ActiveSequenceActor->FindNamedBinding(TEXT("Enemy")).IsValid())	
	{
		if ( IsValid(Context.Target) )
		{
			UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Context.Target);
			
			if (IsValid(TargetASC) )
			{
				TargetASC->CurrentMontageStop(SequenceSettings.MontageBlendOutTime);
			}
			
        	ActiveSequenceActor->SetBindingByTag(TEXT("Enemy"), { Context.Target });	
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

void UYSAbilityPlayback_LockonTarget::ProcessContextBeforePlay()
{
	AActor* ContextInstigator = CapturedContext.Instigator;
	if (IsValid(ContextInstigator))
	{
		if (UYSLockOnComponent* LockOnComponent = UYSLockOnComponent::Get(ContextInstigator))
		{
			CapturedContext.Target = LockOnComponent->GetCurrentTarget();
		}
	}
}

void UYSAbilityPlayback_FirstHitTarget::ProcessContextBeforePlay()
{
	if ( CapturedContext.HitResults.IsEmpty() )
	{
		return;
	}
	
	CapturedContext.Target = CapturedContext.HitResults[0].GetActor();;
}

void UYSAbilityPlaybackBase::DispatchNext(EYSPlaybackResult Result, bool bIsEvaluate)
{
	FYSPlaybackContext NextContext = CapturedContext;
	NextContext.PreviousResult = Result;

	UYSGameplayAbility* Ability = CapturedContext.OwnerAbility.Get();
	if (!IsValid(Ability))
	{
		return;
	}

	for (const FYSPlaybackEdge& Edge : Transitions)
	{
		if (Edge.RequiredResult != Result)
		{
			continue;
		}

		UYSAbilityPlaybackBase* Next = Ability->GetPlaybackNode(Edge.NextNodeIndex);
		if (IsValid(Next) && Next->CheckCondition(NextContext))
		{
			Ability->ActivePlayback(Edge.NextNodeIndex, NextContext);
			return;
		}
	}
	
	// 평가하는 경우에는 단순히 조건 체크 용도로만 사용되고, 실제로 플레이백이 전환되는 것은 아니므로 체인 종료 알림을 보내지 않는다.
	if (!bIsEvaluate)
	{
		Ability->NotifyPlaybackChainFinished();	
	}
}

void UYSAbilityPlaybackBase::OnHit(const TArray<FHitResult>& HitResults)
{	
	CapturedContext.HitResults = HitResults;
	DispatchNext(EYSPlaybackResult::OnHitTarget, true);
}
