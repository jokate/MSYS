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

void UYSAbilityPlaybackBase::SetPlayback(const FYSPlaybackContext& Context)
{
	CapturedContext = Context;
	
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
		UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(CapturedContext.OwnerAbility, 
			TEXT("PlayMontage"), TargetToPlayMontage);
		if ( IsValid(PlayMontageTask))
		{
			PlayMontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontagePlayed);
			PlayMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageInterrupted);
			PlayMontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageInterrupted);
			PlayMontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageInterrupted);
			PlayMontageTask->ReadyForActivation();	
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
	ULevelSequencePlayer* Player = ULevelSequencePlayer::CreateLevelSequencePlayer( this, Sequence , Settings, SequenceActor);
	
	if (!IsValid(Player) || !IsValid(SequenceActor))
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
	
	Player->OnFinished.AddDynamic(this, &ThisClass::OnSequencePlayed);
	Player->Play();
}

void UYSAbilityPlaybackBase::DispatchNext(EYSPlaybackResult Result)
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

		// 종료 시그널
		if (Edge.NextNodeIndex == INDEX_NONE)
		{
			Ability->NotifyPlaybackChainFinished();
			return;
		}

		UYSAbilityPlaybackBase* Next = Ability->GetPlaybackNode(Edge.NextNodeIndex);
		if (IsValid(Next) && Next->CheckCondition(NextContext))
		{
			Ability->ActivePlayback(Edge.NextNodeIndex, NextContext);
			return;
		}
	}

	// 매칭 엣지 없음 = 종료
	Ability->NotifyPlaybackChainFinished();
}

