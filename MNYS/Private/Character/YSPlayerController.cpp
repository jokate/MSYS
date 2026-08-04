// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/YSPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "Character/Components/YSCameraManageComponent.h"
#include "Character/Components/YSTargetingComponent.h"
#include "Character/Components/YSTargetingComponent.h"

class UEnhancedInputLocalPlayerSubsystem;

AYSPlayerController::AYSPlayerController(const FObjectInitializer& ObjectInitializer)
{
	LockOnComponent = CreateDefaultSubobject<UYSCameraManageComponent>(TEXT("LockOnComponent"));
	TargetingComponent = CreateDefaultSubobject<UYSTargetingComponent>(TEXT("TargetingComponent"));
	TargetingComponent = CreateDefaultSubobject<UYSTargetingComponent>(TEXT("TargetingComponent"));
}

void AYSPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(UIInputMappingContext, 0);
	}
}

void AYSPlayerController::AddYawInput(float Val)
{
	/* 컨트롤 로테이션 관련해서 문제가 좀 있을 수 있음 ( 락온 자체가 조금 느낌이 쎄 하단 말이지 )
	if ( IsValid(LockOnComponent) && IsValid(LockOnComponent->GetCurrentTarget()))
	{
		if ( Val <= LockOnBreakThreshold )
		{
			return;
		}
	}
	*/
	
	
	Super::AddYawInput(Val);
}
