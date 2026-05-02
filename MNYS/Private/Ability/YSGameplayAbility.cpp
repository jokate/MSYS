// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/YSGameplayAbility.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

void UYSGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                         const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                         const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	PlayMontage();
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
