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

	FMotionWarpingTarget Target;

	// 만들 수 없으면 걸지 않는다. 앞 노드가 걸어 둔 같은 이름의 타깃도 낡은 값이므로 걷는다.
	if ( GetWarpTargetData(InAbility, Target) == false )
	{
		MotionWarpingComponent->RemoveWarpTarget(MotionWarpTargetName);
		return;
	}

	MotionWarpingComponent->AddOrUpdateWarpTarget(Target);
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

bool FYSMotionWarpParam_ControlRotation::GetWarpTargetData(const UYSGameplayAbility* InAbility, FMotionWarpingTarget& OutTarget) const
{
	AActor* OwnerActor = InAbility->GetOwningActorFromActorInfo();

	if ( IsValid(OwnerActor) == false )
		return false;

	AYSPlayerController* PlayerController = Cast<AYSPlayerController>(OwnerActor->GetInstigatorController());

	if ( IsValid(PlayerController) == false )
		return false;

	OutTarget.Name = MotionWarpTargetName;
	OutTarget.Location = OwnerActor->GetActorLocation();
	OutTarget.Rotation = FRotator(0, PlayerController->GetControlRotation().Yaw, 0);
	return true;
}
// 정책이 변경되어서 Playback 기준으로 변경.
bool FYSMotionWarpParam_Target::GetWarpTargetData(const UYSGameplayAbility* InAbility, FMotionWarpingTarget& OutTarget) const
{
	if ( IsValid(InAbility) == false )
		return false;

	AActor* OwnerActor = InAbility->GetOwningActorFromActorInfo();
	if ( IsValid(OwnerActor) == false )
	{
		return false;
	}
	const TSharedPtr<FYSPlaybackContext>& PlaybackContext = InAbility->GetAbilityPlaybackContext();

	// 몽타주 재생 실패로 어빌리티가 이미 끝났으면 컨텍스트가 비어 있다.
	if ( PlaybackContext.IsValid() == false )
	{
		return false;
	}

	AActor* TargetActor = PlaybackContext->Target;
	if ( IsValid(TargetActor) == false )
	{
		return false;
	}

	const FVector FromAttacker = (OwnerActor->GetActorLocation() - TargetActor->GetActorLocation()).GetSafeNormal2D() * (bFaceTarget ? -1.0f : 1.0f);
	OutTarget.Name = MotionWarpTargetName;
	OutTarget.Rotation = FRotator((FromAttacker).ToOrientationQuat());
	return true;
}
