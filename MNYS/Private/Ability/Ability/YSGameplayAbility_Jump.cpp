// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/Ability/YSGameplayAbility_Jump.h"

#include "GameFramework/Character.h"

UYSGameplayAbility_Jump::UYSGameplayAbility_Jump()
{
	PlaybackType = EYSAbilityPlaybackType::None;
}

// 사실상 점프만 트리거 하고 나머지는 외부 관리가 되는게 좋아보임.
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
