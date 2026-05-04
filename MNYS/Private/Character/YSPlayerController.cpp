// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/YSPlayerController.h"

#include "EnhancedInputSubsystems.h"

class UEnhancedInputLocalPlayerSubsystem;

void AYSPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(UIInputMappingContext, 0);
	}
}
