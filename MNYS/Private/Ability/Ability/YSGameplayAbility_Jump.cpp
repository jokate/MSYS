// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/Ability/YSGameplayAbility_Jump.h"

#include "GameFramework/Character.h"

UYSGameplayAbility_Jump::UYSGameplayAbility_Jump()
{
}

void UYSGameplayAbility_Jump::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	if (ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get()))
	{
		Character->Jump();
	}
	
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
