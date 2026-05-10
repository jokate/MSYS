// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/YSGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "YSAbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Ability/EventAction/YSAbilityEventAction.h"
#include "Character/YSCharacterPlayer.h"
#include "General/YSGameplayTag.h"

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
	
	_SetupPlayMontage();

	AActor* OwnerActor = GetOwningActorFromActorInfo();

	if ( IsValid(OwnerActor) )
	{
		AYSCharacterPlayer* Character = Cast<AYSCharacterPlayer>(OwnerActor);

		if ( IsValid(Character) )
		{
			Character->TransitionStateMachine(ChangeInputStateType);
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

		if ( IsValid(Character) && RuntimeData.IsChainedAbility() == false  )
		{
			Character->TransitionStateMachine(EYSInputStatesType::Idle);
		}
	}
	
	UYSAbilitySystemComponent* YSASC = Cast<UYSAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());

	if ( IsValid(YSASC) )
	{
		YSASC->OnGameplayTagStateChanged.RemoveDynamic(this, &UYSGameplayAbility::OnGameplayTagChanged);
	}

	_ReleaseMotionWarp();

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
