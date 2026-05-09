// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/YSGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "YSAbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
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
		bIsInputAcceptable = bInIsActive;

		// 끝났다면?
		if ( bIsInputAcceptable == false && PendingTransition != nullptr ) 
		{
			UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

			if ( IsValid( ASC ) == false )
				return;
			// 어빌리티 연쇄 트리거.
			bIsChainedAbility = true;
			ASC->TryActivateAbilityByClass(PendingTransition->AbilityClass);	
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
	
	PlayMontage();

	AActor* OwnerActor = GetOwningActorFromActorInfo();

	if ( IsValid(OwnerActor) )
	{
		AYSCharacterPlayer* Character = Cast<AYSCharacterPlayer>(OwnerActor);

		if ( IsValid(Character) )
		{
			Character->TransitionStateMachine(ChangeInputStateType);
		}
	}
}

void UYSGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	AActor* OwnerActor = GetOwningActorFromActorInfo();

	if ( IsValid(OwnerActor) )
	{
		AYSCharacterPlayer* Character = Cast<AYSCharacterPlayer>(OwnerActor);

		if ( IsValid(Character) && bIsChainedAbility == false  )
		{
			Character->TransitionStateMachine(EYSInputStatesType::Idle);
		}
	}

	UYSAbilitySystemComponent* YSASC = Cast<UYSAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());

	if ( IsValid(YSASC) )
	{
		YSASC->OnGameplayTagStateChanged.RemoveDynamic(this, &UYSGameplayAbility::OnGameplayTagChanged);
	}

	PendingTransition = nullptr;
	bIsInputAcceptable = false;
	bIsChainedAbility = false;
}

bool UYSGameplayAbility::TryTransition(const FGameplayTag& InputGameplayTag) 
{
	if ( PendingTransition != nullptr )
	{
		return false;
	}

	if (bIsInputAcceptable == false)
		return false;
	
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

	for ( const FYSComboTransition& Transition : TransitionsByInput )
	{
		if ( Transition.IsTransitionable(InputGameplayTag))
		{
			if ( Transition.bNeedPending )
			{
				PendingTransition = &Transition;
			}
			else
			{
				// 어빌리티 연쇄 트리거.
				bIsChainedAbility = true;
				ASC->TryActivateAbilityByClass(Transition.AbilityClass);
			}
			
			return true;	
		}
	}

	return false;
}

void UYSGameplayAbility::PlayMontage()
{	
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
		else
		{
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
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
