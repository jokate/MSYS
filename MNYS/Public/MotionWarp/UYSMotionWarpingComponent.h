// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MotionWarpingComponent.h"
#include "UYSMotionWarpingComponent.generated.h"


enum class EMotionWarpType : uint8;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MNYS_API UYSMotionWarpingComponent : public UMotionWarpingComponent
{
	GENERATED_BODY()

public :
	static UYSMotionWarpingComponent* GetMotionWarpingComponent(AActor* InActor);
	
protected:

	virtual void SetMotionWarp(const FName InName, EMotionWarpType InMotionWarpType, const float MotionWarpValue = 0.0f);

	virtual void ReleaseMotionWarp(const FName InName);

	virtual void SetMotionWarpToCursorDirection( const FName TargetName, EMotionWarpType InMotionWarpType, const FVector& TargetLocation, const FRotator& TargetRotation );
};
