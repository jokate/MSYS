// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/YSCharacterPlayer.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Input/YSEnhancedInputComponent.h"
#include "Input/StateMachine/YSInputStateMachineComponent.h"


// Sets default values
AYSCharacterPlayer::AYSCharacterPlayer()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 600.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	InputStateMachineComponent = CreateDefaultSubobject<UYSInputStateMachineComponent>(TEXT("InputStateMachine"));
	
}

// Called when the game starts or when spawned
void AYSCharacterPlayer::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AYSCharacterPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AYSCharacterPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UYSEnhancedInputComponent* EnhancedInputComponent = CastChecked<UYSEnhancedInputComponent>(PlayerInputComponent))
	{
		
		//EnhancedInputComponent->BindActionByTag
	}
}

void AYSCharacterPlayer::ProcessInput(const FGameplayTag& InputTag) const
{
	if ( IsValid(InputStateMachineComponent))
	{
		InputStateMachineComponent->AcceptInput(InputTag);
	}
}

