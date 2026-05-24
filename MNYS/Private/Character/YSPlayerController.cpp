// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/YSPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "Character/Components/YSLockOnComponent.h"

class UEnhancedInputLocalPlayerSubsystem;

AYSPlayerController::AYSPlayerController(const FObjectInitializer& ObjectInitializer)
{
	LockOnComponent = CreateDefaultSubobject<UYSLockOnComponent>(TEXT("LockOnComponent"));
}

void AYSPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(UIInputMappingContext, 0);
	}
}
