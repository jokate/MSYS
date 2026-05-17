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
	// 현재 위치 기준으로 돌아가고자 한다면? -> 혹은 타겟을 기준으로 돌아가고자 한다면?
}

void UYSMotionWarpingComponent::ReleaseMotionWarp(const FName InName)
{
	RemoveWarpTarget(InName);
}

// 커서 로케이션 기준으로 모션 워프 설정
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
		break;
    }
}

