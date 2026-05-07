// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/YSAnimInstance.h"


#include "AnimCharacterMovementLibrary.h"
#include "KismetAnimationLibrary.h"
#include "Character/YSCharacterBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UYSAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	AActor* OwnerActor = GetOwningActor();

	if ( IsValid(OwnerActor) == false )
		return;

	AYSCharacterBase* Character = Cast<AYSCharacterBase>(OwnerActor);

	if ( IsValid(Character) == false )
		return;

	CharacterPtr = Character;
	CharMovement = Character->GetCharacterMovement();
}

void UYSAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	UCharacterMovementComponent* TempCharMove = CharMovement.Get();
	AYSCharacterBase* Character = CharacterPtr.Get();
	if ( IsValid(TempCharMove) == false || IsValid(Character) == false )
		return;

	FVector CurAccel = TempCharMove->GetCurrentAcceleration();
	const FVector Multiplicative = FVector(1.f, 1.f, 0.f);
	
	Acceleration2D = CurAccel * Multiplicative;
	bHasAcceleration = !Acceleration2D.IsNearlyZero();

	Velocity = TempCharMove->Velocity;
	LocalVelocity = Character->GetActorTransform().InverseTransformVectorNoScale(Velocity);
 
	Velocity2D = Velocity * Multiplicative;
	bHasVelocity = !Velocity.IsNearlyZero();
	bIsFalling = TempCharMove->IsFalling();

	DisplacementSinceLastUpdate = (Character->GetActorLocation() - CurrentWorldLocation).Size2D();
	CurrentWorldLocation = Character->GetActorLocation();
	DisplacementSpeed = DisplacementSinceLastUpdate * DeltaSeconds;

	UCapsuleComponent* Capsule = Character->GetCapsuleComponent();

	if ( IsValid(Capsule) )
	{
		FRotator Rotation = Capsule->GetComponentRotation();
		LocalVelocityDirectionAngle = UKismetAnimationLibrary::CalculateDirection(Velocity2D, Rotation);
		float AbsLocalVelocityDirection = FMath::Abs(LocalVelocityDirectionAngle);

		if (AbsLocalVelocityDirection <= 65.f )
		{
			VelocityDirection = EVelocityDirection::Frontward;
		}
		else if ( AbsLocalVelocityDirection >= 115.f )
		{
			VelocityDirection = EVelocityDirection::Backward;
		}
		else if ( LocalVelocityDirectionAngle >= 0.f )
		{
			VelocityDirection = EVelocityDirection::Right;
		}
		else
		{
			VelocityDirection = EVelocityDirection::Left;
		}
	}

	FVector StopLocation = UAnimCharacterMovementLibrary::PredictGroundMovementStopLocation(Velocity,TempCharMove->bUseSeparateBrakingFriction, TempCharMove->BrakingFriction, TempCharMove->GroundFriction, TempCharMove->BrakingFrictionFactor, TempCharMove->BrakingDecelerationWalking);

	DistanceToTarget = UKismetMathLibrary::VSizeXY(StopLocation);
}
