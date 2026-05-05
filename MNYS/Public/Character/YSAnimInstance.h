// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "General/YSEnum.h"
#include "YSAnimInstance.generated.h"

class UCharacterMovementComponent;
class AYSCharacterBase;
/**
 * 
 */
UCLASS()
class MNYS_API UYSAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public :
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
public :
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AYSCharacterBase> CharacterPtr;

	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<UCharacterMovementComponent> CharMovement;
	
	UPROPERTY(BLueprintReadOnly)
	FVector Acceleration2D;

	UPROPERTY(BlueprintReadOnly)
	FVector Velocity2D;

	UPROPERTY(BlueprintReadOnly)
	FVector Velocity;

	UPROPERTY(BlueprintReadOnly)
	FVector CurrentWorldLocation;

	UPROPERTY(BlueprintReadOnly)
	float DisplacementSinceLastUpdate = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float DisplacementSpeed = 0.f;
	
	UPROPERTY(BlueprintReadOnly)
	float LocalVelocityDirectionAngle = 0.f;

	UPROPERTY(BlueprintReadOnly)
	bool bHasVelocity = false;

	UPROPERTY(BlueprintReadOnly)
	bool bHasAcceleration = false;
	
	UPROPERTY(BlueprintReadOnly)
	bool bIsFalling = false;
	
	UPROPERTY(BlueprintReadOnly)
	EVelocityDirection VelocityDirection;

	UPROPERTY(BlueprintReadWrite)
	float DistanceToTarget = 0.f;
};
