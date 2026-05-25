// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/YSGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "DefaultLevelSequenceInstanceData.h"
#include "LevelSequence.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "MovieSceneSequencePlaybackSettings.h"
#include "YSAbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Ability/EventAction/YSAbilityEventAction.h"
#include "Character/YSCharacterPlayer.h"
#include "Character/AttributeSet/YSCharacterAttributeSetBase.h"
#include "Character/Components/YSLockOnComponent.h"
#include "General/YSGameplayTag.h"
#include "Library/YSBlueprintFunctionLibrary.h"

UYSGameplayAbility::UYSGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UYSGameplayAbility::OnGameplayTagChanged(const FGameplayTag& Tag, bool bInIsActive)
{
	if ( Tag == YSTags::AcceptAbilityInput )
	{
		RuntimeData.SetInputAcceptable(bInIsActive);

		const FYSComboTransition* Transition = RuntimeData.GetPendingTransition();
		// 끝났다면?
		if ( bInIsActive == false && Transition != nullptr ) 
		{
			UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

			if ( IsValid( ASC ) == false )
				return;
			// 어빌리티 연쇄 트리거.
			RuntimeData.SetChainedAbility(true);
			ASC->TryActivateAbilityByClass(Transition->AbilityClass);	
		}
	}
}

void UYSGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                         const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                         const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	UYSAbilitySystemComponent* YSASC = Cast<UYSAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());

	if ( IsValid(YSASC) )
	{
		YSASC->OnGameplayTagStateChanged.AddDynamic(this, &UYSGameplayAbility::OnGameplayTagChanged);
	}
	
	SetupPlayBack(TriggerEventData);

	AActor* OwnerActor = GetOwningActorFromActorInfo();

	if ( IsValid(OwnerActor) )
	{
		AYSCharacterPlayer* Character = Cast<AYSCharacterPlayer>(OwnerActor);

		if ( IsValid(Character) )
		{
			Character->AddStateToStateMachine(ChangeInputStateType);
		}
	}

	_PrepareForAbilityEvent();
}

void UYSGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	AActor* OwnerActor = GetOwningActorFromActorInfo();

	if ( IsValid(OwnerActor) )
	{
		AYSCharacterPlayer* Character = Cast<AYSCharacterPlayer>(OwnerActor);

		if ( IsValid(Character) && RuntimeData.IsChainedAbility() == false )
		{
			Character->RemoveStateToStateMachine(ChangeInputStateType);
		}
	}
	
	UYSAbilitySystemComponent* YSASC = Cast<UYSAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());

	if ( IsValid(YSASC) )
	{
		YSASC->OnGameplayTagStateChanged.RemoveDynamic(this, &UYSGameplayAbility::OnGameplayTagChanged);
		
		for ( const FActiveGameplayEffectHandle& RuntimeHandle : RuntimeData.GetActiveGameplayEffectHandle() )
		{
			YSASC->RemoveActiveGameplayEffect(RuntimeHandle);	
		}
	}

	if (!RuntimeData.IsChainedAbility())
	{
		_ReleaseMotionWarp();
	}

	RuntimeData.ResetData();
}

bool UYSGameplayAbility::TryTransition(const FGameplayTag& InputGameplayTag) 
{
	if ( RuntimeData.IsPendingTransition() || RuntimeData.IsInputAcceptable() == false ) 
	{
		return false;
	}
	
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

	for ( const FYSComboTransition& Transition : TransitionsByInput )
	{
		if ( Transition.IsTransitionable(InputGameplayTag))
		{
			if ( Transition.bNeedPending )
			{
				RuntimeData.SetPendingTransition(&Transition);
			}
			else
			{
				// 어빌리티 연쇄 트리거.
				RuntimeData.SetChainedAbility(true);
				ASC->TryActivateAbilityByClass(Transition.AbilityClass);
			}
			
			return true;	
		}
	}

	return false;
}

void UYSGameplayAbility::OnTraceComplete(const TArray<FHitResult>& HitResults, const FName& DamageRow)
{
	// 데미지 처리 로직을 여기에서 수행해야 한다.
	UAbilitySystemComponent* OwnerASC = GetAbilitySystemComponentFromActorInfo();
	if ( IsValid(OwnerASC) == false )
		return;

	const UYSCharacterAttributeSetBase* OwnerAttribute = OwnerASC->GetSet<UYSCharacterAttributeSetBase>();
	if ( IsValid(OwnerAttribute) == false )
		return;

	IGenericTeamAgentInterface* TeamAgentInterface = Cast<IGenericTeamAgentInterface>(GetOwningActorFromActorInfo());
	if ( TeamAgentInterface == nullptr )
		return;
		
	for ( const FHitResult& HitResult : HitResults ) 
	{
		AActor* HitActor = HitResult.GetActor();

		if (TeamAgentInterface->GetTeamAttitudeTowards(*HitActor) == ETeamAttitude::Friendly )
			continue;
		
		if ( IsValid(HitActor) == false ) 
			continue;
		
		IYSBattleActor* BattleActor = Cast<IYSBattleActor>(HitActor);
		if (BattleActor == nullptr || BattleActor->IsDead())
		{
			continue;
		}
		IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(HitActor);

		if ( ASI == nullptr )
			continue;
		
		UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
		if ( IsValid(ASC) == false )
			continue;
		
		const UYSCharacterAttributeSetBase* TargetAttributeSet = ASC->GetSet<UYSCharacterAttributeSetBase>();
		if ( IsValid(TargetAttributeSet) == false )
			continue;
		
		// 만약 회피 윈도우에 걸린 경우.
		if (ASC->HasMatchingGameplayTag(YSTags::JustAvoid_Window))
		{
			FGameplayEventData GameplayEventData;
			GameplayEventData.Instigator = GetOwningActorFromActorInfo();
			GameplayEventData.ContextHandle.AddHitResult(HitResult);
			
			UYSLockOnComponent* LockOnComponent = UYSLockOnComponent::Get(HitActor);
			
			if ( IsValid(LockOnComponent) )
			{
				LockOnComponent->ForceSetLockOn(GetOwningActorFromActorInfo());
			}
			
			ASC->HandleGameplayEvent(YSTags::Event_JustAvoid, &GameplayEventData);
			continue;
		}
		
		if (ASC->HasMatchingGameplayTag(YSTags::Invincible))
		{
			continue;
		}
		
		// 데미지 히트에 따른 이벤트 송신.
		UYSBlueprintFunctionLibrary::SendHitEventToTarget(GetOwningActorFromActorInfo(), HitActor, DamageRow);
	}
}

void UYSGameplayAbility::_SetupPlayMontage(const FGameplayEventData* TriggerEventData)
{
	const FYSMontageSelector* CurMontageSelector = MontageSelector.GetPtr<FYSMontageSelector>();

	if ( CurMontageSelector == nullptr )
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	};

	UAnimMontage* TargetToPlayMontage = CurMontageSelector->SelectMontage(this);
	
	if ( IsValid(TargetToPlayMontage) ) 
	{
		UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("PlayMontage"), TargetToPlayMontage);
		if ( IsValid(PlayMontageTask))
		{
			PlayMontageTask->OnCompleted.AddDynamic(this, &UYSGameplayAbility::OnMontagePlayed);
			PlayMontageTask->OnInterrupted.AddDynamic(this, &UYSGameplayAbility::OnMontageInterrupted);
			PlayMontageTask->OnBlendOut.AddDynamic(this, &UYSGameplayAbility::OnMontageInterrupted);
			PlayMontageTask->OnCancelled.AddDynamic(this, &UYSGameplayAbility::OnMontageInterrupted);
			PlayMontageTask->ReadyForActivation();	
		}
	}

	CurMontageSelector->SetMotionWarp(this, true);
}

void UYSGameplayAbility::OnSequencePlayed()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UYSGameplayAbility::_SetupSequence(const FGameplayEventData* TriggerEventData)
{
	// Sequencer가 SkeletalMesh 애니메이션의 완전한 제어권을 갖도록,
	// 진입 전에 AnimInstance 슬롯을 점유 중인 선행 몽타주를 명시적으로 정지한다.
	// CancelAbilitiesWithTags 대신 CurrentMontageStop을 사용하는 이유:
	// 어빌리티 라이프사이클을 건드리지 않고 슬롯 레이어만 클리어하기 위함.
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->CurrentMontageStop(SequenceSettings.MontageBlendOutTime);
	}

	ULevelSequence* Sequence = SequenceSettings.Sequence.LoadSynchronous();
	if (!IsValid(Sequence))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}
	FMovieSceneSequencePlaybackSettings Settings;
	Settings.PlayRate = SequenceSettings.PlayRate;
	Settings.bPauseAtEnd = false;
	ALevelSequenceActor* SequenceActor = nullptr;
	ULevelSequencePlayer* Player = ULevelSequencePlayer::CreateLevelSequencePlayer( this, Sequence , Settings, SequenceActor);
	
	if (!IsValid(Player) || !IsValid(SequenceActor))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}
	
	ActiveSequenceActor = SequenceActor;

	ActiveSequenceActor->SetBindingByTag(TEXT("Player"), { GetOwningActorFromActorInfo() });
	
	UYSLockOnComponent* LockOnComponent = UYSLockOnComponent::Get(GetOwningActorFromActorInfo());
	
	if ( IsValid(LockOnComponent) && ActiveSequenceActor->FindNamedBinding(TEXT("Enemy")).IsValid())	
	{
		AActor* LockOnTarget = LockOnComponent->GetCurrentTarget();
		if ( IsValid(LockOnTarget) )
		{
			UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(LockOnTarget);
			
			if (IsValid(TargetASC) )
			{
				TargetASC->CurrentMontageStop(SequenceSettings.MontageBlendOutTime);
			}
			
        	ActiveSequenceActor->SetBindingByTag(TEXT("Enemy"), { LockOnComponent->GetCurrentTarget() });	
		}
	}
	
	UDefaultLevelSequenceInstanceData* DefaultInstanceData = Cast<UDefaultLevelSequenceInstanceData>(SequenceActor->DefaultInstanceData);
	
	if ( IsValid(DefaultInstanceData) )
	{
		DefaultInstanceData->TransformOriginActor = GetOwningActorFromActorInfo();
		DefaultInstanceData->TransformOrigin = GetOwningActorFromActorInfo()->GetActorTransform();
	}
	
	if (!SequenceSettings.bOverrideCameraBySequence && IsValid(ActiveSequenceActor))
	{
		// 카메라 컷 트랙 비활성화
		ActiveSequenceActor->bOverrideInstanceData = false;
	}
	
	Player->OnFinished.AddDynamic(this, &UYSGameplayAbility::OnSequencePlayed);
	Player->Play();
}

void UYSGameplayAbility::_ReleaseMotionWarp() const
{
	const FYSMontageSelector* CurMontageSelector = MontageSelector.GetPtr<FYSMontageSelector>();

	if ( CurMontageSelector == nullptr )
	{
		return;
	};

	CurMontageSelector->SetMotionWarp(this, false);
}

void UYSGameplayAbility::_ProcessEvent(FGameplayEventData Payload)
{
	if (EventActionMap.Contains(Payload.EventTag) == false )
		return;
	const TArray<UYSAbilityEventAction*>& EventActions = EventActionMap[Payload.EventTag].GetEventActions();

	for ( UYSAbilityEventAction* Action : EventActions )
	{
		if ( IsValid(Action) )
		{
			Action->Execute(this, Payload);;
		}
	}
}

void UYSGameplayAbility::_PrepareForAbilityEvent()
{
	for ( auto& EventAction : EventActionMap )
	{
		UAbilityTask_WaitGameplayEvent* Task = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, EventAction.Key, nullptr, false);

		if ( IsValid(Task) )
		{
			Task->EventReceived.AddDynamic(this, &UYSGameplayAbility::_ProcessEvent);
			Task->ReadyForActivation();
		}
	}
}

void UYSGameplayAbility::OnMontagePlayed()
{
	bool bReplicatedEndAbility = true;
	bool bWasCancelled = true;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicatedEndAbility, bWasCancelled);
}

void UYSGameplayAbility::OnMontageInterrupted()
{
	bool bReplicatedEndAbility = true;
	bool bWasCancelled = false;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicatedEndAbility, bWasCancelled);
}

void UYSGameplayAbility::SetupPlayBack(const FGameplayEventData* TriggerEventData)
{
	switch ( PlaybackType )
	{
	case EYSAbilityPlaybackType::Montage :
		_SetupPlayMontage();
		break;
	case EYSAbilityPlaybackType::Sequence :
		_SetupSequence(TriggerEventData);
		break;
	default :
		break;
	}
}

#if UE_EDITOR
void UYSGameplayAbility::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	
	if ( PropertyName == GET_MEMBER_NAME_CHECKED(UYSGameplayAbility, AbilityType) )
	{
		EventActionMap.Empty();
		
		switch (AbilityType )
		{
		case EYSAbilityType::None:
			break;
		case EYSAbilityType::MeleeAttack :
			{
				FYSEventPayload TraceStart;
				TraceStart.EventActions.Add(NewObject<UYSAbilityEventAction_StartTrace>(this));
				EventActionMap.Add(YSTags::Event_TraceStart, TraceStart);
			
				FYSEventPayload TraceEnd;
				TraceEnd.EventActions.Add(NewObject<UYSAbilityEventAction_StopTrace>(this));
				EventActionMap.Add(YSTags::Event_TraceEnd, TraceEnd);
				break;
			}
		case EYSAbilityType::RangedAttack : 
			break;
		default: 
			break;
		}
	}
}
#endif
