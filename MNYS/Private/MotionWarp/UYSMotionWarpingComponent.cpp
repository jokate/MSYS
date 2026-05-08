// Fill out your copyright notice in the Description page of Project Settings.


#include "MotionWarp/UYSMotionWarpingComponent.h"

#include "General/YSEnum.h"



UYSMotionWarpingComponent* UYSMotionWarpingComponent::GetMotionWarpingComponent(AActor* InActor)
{
	UYSMotionWarpingComponent* Component = InActor->FindComponentByClass<UYSMotionWarpingComponent>();
	return Component;
}

void UYSMotionWarpingComponent::SetMotionWarp(const FName InName, EMotionWarpType InMotionWarpType,
                                              const float MotionWarpValue)
{
	// 정책 결정후에 정하기로..
	
}

void UYSMotionWarpingComponent::ReleaseMotionWarp(const FName InName)
{
	RemoveWarpTarget(InName);
}

void UYSMotionWarpingComponent::SetMotionWarpToCursorDirection(const FName TargetName, EMotionWarpType InMotionWarpType,
	const FVector& TargetLocation, const FRotator& TargetRotation)
{
	switch (InMotionWarpType)
    {
    case EMotionWarpType::TranslationAndRotation:
    	AddOrUpdateWarpTargetFromLocationAndRotation(TargetName,  TargetLocation, TargetRotation);
    	break;
    case EMotionWarpType::RotationOnly:
    	AddOrUpdateWarpTargetFromLocationAndRotation(TargetName,FVector::ZeroVector,TargetRotation);
    	break;
    case EMotionWarpType::TranslationOnly:
    	AddOrUpdateWarpTargetFromLocationAndRotation(TargetName,  TargetLocation,FRotator::ZeroRotator);
    	break;
	default :
    }
}

