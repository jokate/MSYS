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
#include "Character/Components/YSCameraLockOnComponent.h"

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
	
	// 기존에 받은 정보들은 그냥 없앰 처리.
	CapturedContext.ContextTags.Reset();
}

void UYSAbilityPlaybackBase::OnSequencePlayed()
{
	DispatchNext(EYSPlaybackEvent::Completed);
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

bool UYSAbilityPlaybackBase::TryAcceptContextTag(const FGameplayTag& InputTag)
{
	// 1단계: 입력 태그 추가
	CapturedContext.ContextTags.AddTag(InputTag);
	
	// 2단계: 즉시 전환이 활성화된 경우, 실제로 전환 가능한지 평가
	if ( bImmediateTransition )
	{
		// 실제 전환 시도 (bIsEvaluate=false로 실제 전환 수행)
		return DispatchNext(EYSPlaybackEvent::OnCheckContextTag, false);
	}
	
	// 3단계: 즉시 전환 미활성화 상태 → 조건만 평가
	// 나중에 실제 전환이 필요할 때를 위해 평가만 수행
	for (const FYSPlaybackEdge& Edge : Transitions)
	{
		if (Edge.RequiredResult == EYSPlaybackEvent::OnCheckContextTag)
		{
			bool bConditionPassed = true;
			for ( const FInstancedStruct& InstancedStruct : Edge.TransitionConditions )
			{
				const FYSPlaybackCondition* Condition = InstancedStruct.GetPtr<FYSPlaybackCondition>();
			
				if ( !Condition )
				{
					continue;
				}
			
				bConditionPassed &= Condition->Evaluate(CapturedContext);
			
				if ( !bConditionPassed )
				{
					break;
				}
			}
			
			if (bConditionPassed && CapturedContext.OwnerAbility->GetPlaybackNode(Edge.NextNodeIndex) != nullptr)
			{
				return true;
			}
		}
	}
	
	return false;  // 전환 불가
}

void UYSAbilityPlaybackBase::OnMontagePlayed()
{
	DispatchNext(EYSPlaybackEvent::Completed);
}

void UYSAbilityPlaybackBase::OnMontageInterrupted()
{
	DispatchNext(EYSPlaybackEvent::Interrupted);
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

void UYSAbilityPlayback_FirstHitTarget::ProcessContextBeforePlay()
{
	UYSGameplayAbility* OwnerAbility = CapturedContext.OwnerAbility;
	 
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
	
	CapturedContext.Target = HitActors[0];
}

void UYSAbilityPlayback_ReleaseBuff::ProcessContextBeforePlay()
{
	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(CapturedContext.Instigator))
	{
		ASC->RemoveActiveEffectsWithTags(BuffTags);
	}
}

bool UYSAbilityPlaybackBase::DispatchNext(EYSPlaybackEvent Event, bool bIsEvaluate)
{
	FYSPlaybackContext NextContext = CapturedContext;
	NextContext.PreviousResult = Event;

	UYSGameplayAbility* Ability = CapturedContext.OwnerAbility.Get();
	if (!IsValid(Ability))
	{
		return true;
	}

	for (const FYSPlaybackEdge& Edge : Transitions)
	{
		if (Edge.RequiredResult != Event)
		{
			continue;
		}
		
		bool bConditionPassed = true;
		for ( const FInstancedStruct& InstancedStruct : Edge.TransitionConditions )
		{
			const FYSPlaybackCondition* Condition = InstancedStruct.GetPtr<FYSPlaybackCondition>();
			
			if ( !Condition )
			{
				continue;
			}
			
			bConditionPassed &= Condition->Evaluate(CapturedContext);
			
			 if ( !bConditionPassed )
			 {
			 	break;
			 }
		}
		
		UYSAbilityPlaybackBase* Next = Ability->GetPlaybackNode(Edge.NextNodeIndex);
		
		if (bConditionPassed && IsValid(Next))
		{
			Ability->ActivePlayback(Edge.NextNodeIndex, NextContext);
			return true;
		}
	}
	
	// 평가하는 경우에는 단순히 조건 체크 용도로만 사용되고, 실제로 플레이백이 전환되는 것은 아니므로 체인 종료 알림을 보내지 않는다.
	if (!bIsEvaluate)
	{
		Ability->NotifyPlaybackChainFinished();	
		return false;
	}
	
	return false;
}

void UYSAbilityPlaybackBase::OnHit(const TArray<FHitResult>& HitResults)
{	
	DispatchNext(EYSPlaybackEvent::OnHitTarget, true);
}
