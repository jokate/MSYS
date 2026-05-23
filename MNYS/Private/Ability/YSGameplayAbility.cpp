// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/YSGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "YSAbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Ability/EventAction/YSAbilityEventAction.h"
#include "Character/YSCharacterPlayer.h"
#include "Character/AttributeSet/YSCharacterAttributeSetBase.h"
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
	
	if ( bShouldPlayMontage )
		_SetupPlayMontage();

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
		IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(HitActor);

		if ( ASI == nullptr )
			continue;
		
		UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
		if ( IsValid(ASC) == false )
			continue;
		
		const UYSCharacterAttributeSetBase* TargetAttributeSet = ASC->GetSet<UYSCharacterAttributeSetBase>();

		if ( IsValid(TargetAttributeSet) == false )
			continue;
		
		float CurHp = TargetAttributeSet->GetCurrentHp();
		
		if ( CurHp <= 0.f )
			continue;
		
		// 만약 회피 윈도우에 걸린 경우.
		if (ASC->HasMatchingGameplayTag(YSTags::JustAvoid_Window))
		{
			ASC->HandleGameplayEvent(YSTags::Event_JustAvoid, nullptr);
			continue;
		}
		
		if (ASC->HasMatchingGameplayTag(YSTags::Invincible))
		{
			continue;
		}
		
		// 스킬 별 데미지 관련 정책은 필요함. (다만 현재 기준 정책은 없는 부분이라서 어떻게 해야할 지 고민 좀 해봐야 할 듯 싶음)
		// 아마 해당 부분은 Static 하게 결정되어야 할 듯 싶습니다.
		// EX ) 기본 공격 * ( 계수 ( 1 + Extra ) ) + 추가 데미지 (버프..?) - 방어력 비례 이런 형태. (수식은 뭐 간단하지만 어느정도의 감각이 있어야 할 듯.)
		float FinalDamage = UYSBlueprintFunctionLibrary::GetFinalDamage(OwnerAttribute, TargetAttributeSet, DamageRow);
		
		// 데미지 히트에 따른 이벤트 송신.
		UYSBlueprintFunctionLibrary::SendHitEventToTarget(GetOwningActorFromActorInfo(), ASC, FinalDamage, DamageRow);
		
		// 데미지 처리 로직 추가. ( 클램핑은 내부에서 알아 처리 될 거임 )
		ASC->SetNumericAttributeBase(TargetAttributeSet->GetCurrentHpAttribute(), CurHp - FinalDamage);	
	}
}

void UYSGameplayAbility::_SetupPlayMontage()
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
