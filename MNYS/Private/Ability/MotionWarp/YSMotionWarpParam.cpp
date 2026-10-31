// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/MotionWarp/YSMotionWarpParam.h"

#include "AnimNotifyState_MotionWarping.h"
#include "RootMotionModifier.h"
#include "Ability/YSGameplayAbility.h"
#include "Ability/AbilityComponent/YSAbilityPlayback.h"
#include "Animation/AnimMontage.h"
#include "Character/YSPlayerController.h"
#include "MotionWarp/UYSMotionWarpingComponent.h"

namespace
{
	TSet<FString> WarnedMontageWarps;
}

void FYSMotionWarpParam::WarnIfMontageCannotWarp(const UYSGameplayAbility* InAbility) const
{
	const UAnimMontage* Montage = InAbility->GetCurrentMontage();

	if ( IsValid(Montage) == false )
		return;

	const TCHAR* Problem = nullptr;

	// 루트 모션이 없으면 엔진이 워프 수정자를 돌리지 않는다. 구간이 있어도 소용없다.
	if ( Montage->HasRootMotion() == false )
	{
		Problem = TEXT("애니 루트 모션이 꺼져 워프가 걸리지 않는다");
	}
	else
	{
		TArray<FMotionWarpingWindowData> Windows;
		UMotionWarpingUtilities::GetMotionWarpingWindowsForWarpTargetFromAnimation(Montage, MotionWarpTargetName, Windows);

		const bool bHasTranslation = Windows.ContainsByPredicate([](const FMotionWarpingWindowData& Window)
		{
			const URootMotionModifier_Warp* Modifier = IsValid(Window.AnimNotify) ? Cast<URootMotionModifier_Warp>(Window.AnimNotify->RootMotionModifier) : nullptr;
			return IsValid(Modifier) && Modifier->bWarpTranslation;
		});

		if ( Windows.Num() == 0 )
		{
			Problem = TEXT("같은 이름의 워프 구간이 없다");
		}
		else if ( RequiresTranslation() && bHasTranslation == false )
		{
			Problem = TEXT("워프 구간의 이동이 꺼져 있다");
		}
	}

	if ( Problem == nullptr )
		return;

	bool bAlreadyWarned = false;
	WarnedMontageWarps.Add(Montage->GetPathName() + TEXT("|") + MotionWarpTargetName.ToString(), &bAlreadyWarned);

	if ( bAlreadyWarned == false )
	{
		UE_LOG(LogTemp, Warning, TEXT("[MotionWarp] %s: 몽타주 %s — %s (워프 타깃 %s)."),
			*InAbility->GetName(), *Montage->GetName(), Problem, *MotionWarpTargetName.ToString());
	}
}

void FYSMotionWarpParam::ResolveMotionWarp(const UYSGameplayAbility* InAbility) const
{
	AActor* OwnerActor = InAbility->GetOwningActorFromActorInfo();

	if ( IsValid(OwnerActor) == false )
		return;

	UYSMotionWarpingComponent* MotionWarpingComponent = UYSMotionWarpingComponent::GetMotionWarpingComponent(OwnerActor);

	if (IsValid(MotionWarpingComponent) == false )
		return;

	WarnIfMontageCannotWarp(InAbility);

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

bool FYSMotionWarpParam_TargetFront::GetWarpTargetData(const UYSGameplayAbility* InAbility, FMotionWarpingTarget& OutTarget) const
{
	if ( IsValid(InAbility) == false )
		return false;

	AActor* OwnerActor = InAbility->GetOwningActorFromActorInfo();
	const TSharedPtr<FYSPlaybackContext>& PlaybackContext = InAbility->GetAbilityPlaybackContext();
	AActor* TargetActor = PlaybackContext.IsValid() ? PlaybackContext->Target.Get() : nullptr;

	if ( IsValid(OwnerActor) == false || IsValid(TargetActor) == false )
		return false;

	const FVector From = OwnerActor->GetActorLocation();
	FVector ToTarget = TargetActor->GetActorLocation() - From;
	ToTarget.Z = 0.f;

	const FVector Direction = ToTarget.GetSafeNormal();
	const float Travel = FMath::Clamp(ToTarget.Size() - StopDistance, 0.f, MaxDistance);

	OutTarget.Name = MotionWarpTargetName;
	OutTarget.Location = From + Direction * Travel;
	OutTarget.Rotation = Direction.Rotation();
	return true;
}
