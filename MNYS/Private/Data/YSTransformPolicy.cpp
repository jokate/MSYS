// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/YSTransformPolicy.h"

#include "NavigationSystem.h"
#include "Character/YSCharacterBase.h"
#include "Character/Components/YSCameraManageComponent.h"
#include "Character/Components/YSTargetingComponent.h"

FVector FYSLocationPolicy_Socket::GetLocation(const FYSTransformPolicyContext& Context) const
{
	AYSCharacterBase* Character = Cast<AYSCharacterBase>(Context.OwnerActor);
	if (IsValid(Character) && IsValid(Character->GetMesh()) && SocketName != NAME_None)
	{
		return Character->GetMesh()->GetSocketLocation(SocketName);
	}
	return Context.OwnerActor->GetActorLocation();
}

FVector FYSLocationPolicy_RelativeOffset::GetLocation(const FYSTransformPolicyContext& Context) const
{
	return Context.OwnerActor->GetActorTransform().TransformPosition(RelativeOffset);
}

FVector FYSLocationPolicy_RandomOffset::GetLocation(const FYSTransformPolicyContext& Context) const
{
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(Context.OwnerActor);
	if ( IsValid(NavSystem) == false )
	{
		return FVector::ZeroVector;
	}
			
	FNavLocation RetLocation;
	NavSystem->GetRandomPointInNavigableRadius(Context.OwnerActor->GetActorLocation(), RandomOffsetRange.Size(), RetLocation);
	return RetLocation;
}

FVector FYSLocationPolicy_Targeting::GetLocation(const FYSTransformPolicyContext& Context) const
{
	UYSTargetingComponent* TargetingComponent = UYSTargetingComponent::Get(Context.OwnerActor);
	if ( IsValid(TargetingComponent) == false )
	{
		return FVector::ZeroVector;
	}
			
	return TargetingComponent->GetResult().Location;
}

FVector FYSLocationPolicy_HitResult::GetLocation(const FYSTransformPolicyContext& Context) const
{
	if (Context.HitResult == nullptr)
	{
		return Super::GetLocation(Context);
	}
	return Context.HitResult->ImpactPoint;
}

FRotator FYSRotationPolicy_Socket::GetRotation(const FYSTransformPolicyContext& Context) const
{
	AYSCharacterBase* Character = Cast<AYSCharacterBase>(Context.OwnerActor);
	if (IsValid(Character) && IsValid(Character->GetMesh()))
	{
		if (SocketName != NAME_None)
			return Character->GetMesh()->GetSocketRotation(SocketName);
	}
	return Super::GetRotation(Context);
}

FRotator FYSRotationPolicy_Control::GetRotation(const FYSTransformPolicyContext& Context) const
{
	if (const AController* Controller = Context.OwnerActor->GetInstigatorController())
		return Controller->GetControlRotation();

	return  Super::GetRotation(Context);
}

FRotator FYSRotationPolicy_LockOnTarget::GetRotation(const FYSTransformPolicyContext& Context) const
{
	if (UYSCameraManageComponent* LockOn = UYSCameraManageComponent::Get(Context.OwnerActor))
	{
		if (AActor* Target = LockOn->GetCurrentTarget())
		{
			const FVector Dir = (Target->GetActorLocation() - Context.OwnerActor->GetActorLocation()).GetSafeNormal();
			return Dir.Rotation();
		}
	}
	return Super::GetRotation(Context);
}

FRotator FYSRotationPolicy_PlaybackTarget::GetRotation(const FYSTransformPolicyContext& Context) const
{
	if (IsValid(Context.TargetActor))
	{
		const FVector Dir = (Context.TargetActor->GetActorLocation() - Context.OwnerActor->GetActorLocation()).GetSafeNormal();
		return Dir.Rotation();
	}
	return Super::GetRotation(Context);
}

FRotator FYSRotationPolicy_RelativeOffset::GetRotation(const FYSTransformPolicyContext& Context) const
{
	return Context.OwnerActor->GetActorTransform().TransformRotation(RelativeOffset.Quaternion()).Rotator();
}

FRotator FYSRotationPolicy_Targeting::GetRotation(const FYSTransformPolicyContext& Context) const
{
	UYSTargetingComponent* TargetingComponent = UYSTargetingComponent::Get(Context.OwnerActor);
	if ( IsValid(TargetingComponent) == false )
	{
		return Super::GetRotation(Context);
	}
			
	return TargetingComponent->GetResult().Direction.Rotation();
}

FRotator FYSRotationPolicy_TargetingAimPoint::GetRotation(const FYSTransformPolicyContext& Context) const
{
	const UYSTargetingComponent* TargetingComponent = UYSTargetingComponent::Get(Context.OwnerActor);
	if ( IsValid(TargetingComponent) == false || TargetingComponent->IsTargeting() == false )
	{
		return Super::GetRotation(Context);
	}

	const FVector ToAim = TargetingComponent->GetResult().AimPoint - Context.ResolvedLocation;
	return ToAim.IsNearlyZero() ? Super::GetRotation(Context) : ToAim.Rotation();
}

FTransform FYSTransformPolicy::GetFinalTransform(const FYSTransformPolicyContext& Context) const
{
	const FYSLocationPolicyBase* TmpLocationPolicyBase = LocationPolicy.GetPtr<FYSLocationPolicyBase>();

	FVector FinalLocation = TmpLocationPolicyBase ? TmpLocationPolicyBase->GetLocation(Context) : FVector::ZeroVector;

	FYSTransformPolicyContext RotationContext = Context;
	RotationContext.ResolvedLocation = FinalLocation;

	const FYSRotationPolicyBase* TmpRotationPolicyBase = RotationPolicy.GetPtr<FYSRotationPolicyBase>();

	FRotator FinalRotation = TmpRotationPolicyBase ? TmpRotationPolicyBase->GetRotation(RotationContext) : FRotator::ZeroRotator;

	return FTransform(FinalRotation, FinalLocation);
}
