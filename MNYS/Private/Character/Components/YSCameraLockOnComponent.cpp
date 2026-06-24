// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Components/YSCameraLockOnComponent.h"

#include "YSAbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Character/YSCharacterPlayer.h"
#include "Character/YSPlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "General/YSGameplayTag.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Library/YSBlueprintFunctionLibrary.h"


// Sets default values for this component's properties
UYSCameraLockOnComponent::UYSCameraLockOnComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

UYSCameraLockOnComponent* UYSCameraLockOnComponent::Get(const AActor* Character)
{	
	if ( IsValid(Character) == false )
		return nullptr;
	
	AYSPlayerController* PC = Character->GetInstigatorController<AYSPlayerController>();
	
	if ( IsValid(PC) == false ) 
		return nullptr;
	
	return PC->LockOnComponent;
}


// Called when the game starts
void UYSCameraLockOnComponent::BeginPlay()
{
	Super::BeginPlay();

	AYSPlayerController* PlayerController = Cast<AYSPlayerController>(GetOwner());

	if ( IsValid(PlayerController) )
	{
		OwnerPlayer = Cast<AYSCharacterPlayer>(PlayerController->GetPawn());
		OwnerPlayerController = PlayerController;
	}

	if ( OwnerPlayer.IsValid() )
	{
		if ( USpringArmComponent* Boom = OwnerPlayer->GetCameraBoom() )
		{
			DefaultArmLength     = Boom->TargetArmLength;
			DefaultSocketOffset = Boom->SocketOffset;
		}
		if ( UCameraComponent* Cam = OwnerPlayer->GetFollowCamera() )
		{
			DefaultFOV = Cam->FieldOfView;
			DefaultRotation = Cam->GetRelativeRotation();
		}
	}
}


// Called every frame
void UYSCameraLockOnComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ProcessLockOnFunction(DeltaTime);
	TickCameraEffect(DeltaTime);
}

void UYSCameraLockOnComponent::ForceSetLockOn(AActor* TargetToLockOn)
{
	CurrentLockedTarget = TargetToLockOn;

	FRotator CharacterRotation = UYSBlueprintFunctionLibrary::GetEventRotation(EYSDirectionPolicy::UseTowardLockOnTarget, OwnerPlayer.Get(), NAME_None, FRotator::ZeroRotator);
	OwnerPlayer->SetActorRotation(CharacterRotation);
}

void UYSCameraLockOnComponent::ProcessLockOnFunction(float DeltaTime)
{
	if ( OwnerPlayer.IsValid() == false )
		return;
	
	if ( CurrentLockedTarget.IsValid() )
	{
		ChaseCamera(DeltaTime);
		return;
	}
}

void UYSCameraLockOnComponent::ChaseCamera(float DeltaTime)
{
	if ( OwnerPlayerController.IsValid() == false || CurrentLockedTarget.IsValid() == false )
		return;
	
	FVector ToTarget = (CurrentLockedTarget->GetActorLocation() - OwnerPlayer->GetActorLocation()).GetSafeNormal();
	FRotator TargetRot  = ToTarget.Rotation();
	FRotator CurrentRot = OwnerPlayerController->GetControlRotation();
	
	TargetRot.Pitch = CurrentRot.Pitch;
	OwnerPlayerController->SetControlRotation(
		FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, CameraInterpSpeed));
}
void UYSCameraLockOnComponent::StartCameraEffect(const FYSCameraEffectParams& Params)
{
	ActiveCameraParams  = Params;
	bCameraEffectActive = true;
	bCameraRestoring    = false;
}

void UYSCameraLockOnComponent::StopCameraEffect()
{
	bCameraEffectActive = false;
	bCameraRestoring    = true;
}

void UYSCameraLockOnComponent::TickCameraEffect(float DeltaTime)
{
	if ( !bCameraEffectActive && !bCameraRestoring )
		return;

	if ( OwnerPlayer.IsValid() == false )
		return;

	USpringArmComponent* Boom = OwnerPlayer->GetCameraBoom();
	UCameraComponent* Cam  = OwnerPlayer->GetFollowCamera();

	if ( !IsValid(Boom) || !IsValid(Cam) )
		return;
	
	
	const float TargetArm  = bCameraEffectActive ? ActiveCameraParams.TargetArmLength : DefaultArmLength;
	const FVector TargetOff = bCameraEffectActive ? ActiveCameraParams.SocketOffset   : DefaultSocketOffset;
	const float TargetFOV  = bCameraEffectActive ? ActiveCameraParams.FieldOfView     : DefaultFOV;
	const float Speed      = bCameraEffectActive ? ActiveCameraParams.InterpInSpeed   : ActiveCameraParams.InterpOutSpeed;
	const FRotator Rotator = bCameraEffectActive ? ActiveCameraParams.RelativeRotator : DefaultRotation;
	
	Boom->TargetArmLength = FMath::FInterpTo(Boom->TargetArmLength, TargetArm,  DeltaTime, Speed);
	Boom->SocketOffset  = FMath::VInterpTo(Boom->SocketOffset,  TargetOff, DeltaTime, Speed);
	Cam->SetRelativeRotation(FMath::RInterpTo(Cam->GetRelativeRotation(), Rotator, DeltaTime, Speed));
	Cam->FieldOfView = FMath::FInterpTo(Cam->FieldOfView, TargetFOV, DeltaTime, Speed);

	if ( bCameraRestoring )
	{
		const bool bArmDone = FMath::IsNearlyEqual(Boom->TargetArmLength, DefaultArmLength, 1.f);
		const bool bFOVDone = FMath::IsNearlyEqual(Cam->FieldOfView, DefaultFOV, 0.5f);

		if ( bArmDone && bFOVDone )
		{
			Boom->TargetArmLength = DefaultArmLength;
			Boom->SocketOffset  = DefaultSocketOffset;
			Cam->FieldOfView      = DefaultFOV;
			bCameraRestoring      = false;
		}
	}
}

