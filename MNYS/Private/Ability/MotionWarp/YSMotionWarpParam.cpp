// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/MotionWarp/YSMotionWarpParam.h"

#include "Ability/YSGameplayAbility.h"
#include "Ability/AbilityComponent/YSAbilityPlayback.h"
#include "Character/YSPlayerController.h"
#include "MotionWarp/UYSMotionWarpingComponent.h"

void FYSMotionWarpParam::ResolveMotionWarp(const UYSGameplayAbility* InAbility) const
{
	AActor* OwnerActor = InAbility->GetOwningActorFromActorInfo();
	
	if ( IsValid(OwnerActor) == false )
		return;
	
	UYSMotionWarpingComponent* MotionWarpingComponent = UYSMotionWarpingComponent::GetMotionWarpingComponent(OwnerActor);
	
	if (IsValid(MotionWarpingComponent) == false )
		return;
	
	MotionWarpingComponent->AddOrUpdateWarpTarget(GetWarpTargetData(InAbility));
}

void FYSMotionWarpParam::ReleaseMotionWarp(const UYSGameplayAbility* InAbility) const
{
	AActor* OwnerActor = InAbility->GetOwningActorFromActorInfo();
	
	if ( IsValid(OwnerActor) == false )
		return;
	
	UYSMotionWarpingComponent* MotionWarpingComponent = UYSMotionWarpingComponent::GetMotionWarpingComponent(OwnerActor);
	
	if (IsValid(MotionWarpingComponent) == false )
		return;
	
	MotionWarpingComponent->RemoveWarpTarget(MotionWarpTargetName);
}

FMotionWarpingTarget FYSMotionWarpParam_ControlRotation::GetWarpTargetData(const UYSGameplayAbility* InAbility) const
{
	FMotionWarpingTarget Target;
	Target.Name = MotionWarpTargetName;
	AActor* OwnerActor = InAbility->GetOwningActorFromActorInfo();
		
	if ( IsValid(OwnerActor) == false )
		return Target;
	
	AYSPlayerController* PlayerController = Cast<AYSPlayerController>(OwnerActor->GetInstigatorController());
	
	if ( IsValid(PlayerController) == false )
		return Target;
	
	Target.Location = OwnerActor->GetActorLocation();
	Target.Rotation = FRotator(0, PlayerController->GetControlRotation().Yaw, 0);
	return Target;
}
// 정책이 변경되어서 Playback 기준으로 변경.
FMotionWarpingTarget FYSMotionWarpParam_Target::GetWarpTargetData(const UYSGameplayAbility* InAbility) const
{
	FMotionWarpingTarget Target;
	Target.Name = MotionWarpTargetName;
	
	if ( IsValid(InAbility) == false )
		return Target;
	
	
	AActor* OwnerActor = InAbility->GetOwningActorFromActorInfo();
	if ( IsValid(OwnerActor) == false )
	{
		return Target;
	}
	const TSharedPtr<FYSPlaybackContext>& PlaybackContext = InAbility->GetAbilityPlaybackContext();
	
	AActor* TargetActor = PlaybackContext->Target;
	if ( IsValid(TargetActor) == false )
	{
		return Target;
	}
	
	const FVector FromAttacker = (OwnerActor->GetActorLocation() - TargetActor->GetActorLocation()).GetSafeNormal2D() * (bFaceTarget ? -1.0f : 1.0f);
	Target.Rotation = FRotator((FromAttacker).ToOrientationQuat());
	return Target;
}
