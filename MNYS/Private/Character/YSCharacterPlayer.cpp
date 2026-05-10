// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/YSCharacterPlayer.h"

#include "EnhancedInputSubsystems.h"
#include "YSAbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Character/Components/YSCharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "General/YSGameplayTag.h"
#include "General/YSInputGameplayTags.h"
#include "Input/YSEnhancedInputComponent.h"
#include "Input/StateMachine/YSInputStateMachineComponent.h"


class UEnhancedInputLocalPlayerSubsystem;
// Sets default values
AYSCharacterPlayer::AYSCharacterPlayer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UYSCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;
	
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

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f);
	
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

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(PlayerInputMappingContext, 0);
		}
	}
	
	if (UYSEnhancedInputComponent* EnhancedInputComponent = CastChecked<UYSEnhancedInputComponent>(PlayerInputComponent))
	{
		if (IsValid(InputConfig) == false )
		{
			return;
		}
		
		for (const FTaggedInputAction& InputAction : InputConfig->TaggedInputActions )
		{
			if ( InputAction.InputTag == YSInputTags::InputMove )
			{
				EnhancedInputComponent->BindActionByTag(InputConfig, InputAction.InputTag, ETriggerEvent::Triggered, this, &AYSCharacterPlayer::Move);
				EnhancedInputComponent->BindActionByTag(InputConfig, InputAction.InputTag, ETriggerEvent::Completed, this, &AYSCharacterPlayer::OnMovementComplete);
			}
			else if ( InputAction.InputTag == YSInputTags::InputLook )
			{
				EnhancedInputComponent->BindActionByTag(InputConfig, InputAction.InputTag, ETriggerEvent::Triggered, this, &AYSCharacterPlayer::Look);
			}
			else
			{
				EnhancedInputComponent->BindActionByTag(InputConfig, InputAction.InputTag, ETriggerEvent::Triggered, this, &AYSCharacterPlayer::ProcessInput, InputAction.InputTag);	
			}
		}
	}
}

void AYSCharacterPlayer::ProcessInput(FGameplayTag InputTag)
{
	if ( IsValid(InputStateMachineComponent))
	{
		InputStateMachineComponent->AcceptInput(InputTag);
	}
}

void AYSCharacterPlayer::TransitionStateMachine(EYSInputStatesType InputStateType) const
{
	if ( IsValid(InputStateMachineComponent) )
	{
		InputStateMachineComponent->TransitionState(InputStateType);
	}
}

void AYSCharacterPlayer::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	
	if ( Controller != nullptr )
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AYSCharacterPlayer::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	if ( IsValid( GetYSCharacterMovement() ) == false )
	{
		return;
	}

	// 입력에 의한 움직임은 제어
	if ( GetYSCharacterMovement()->IsMovementBlocked() )
		return;
	
	FVector2D MovementVector = Value.Get<FVector2D>();

	ProcessMovementInput(MovementVector.GetSafeNormal());
	
	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
			
		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AYSCharacterPlayer::OnMovementComplete()
{
	LastDirectionTags.Empty();
}

void AYSCharacterPlayer::SetMovementBlocked(bool bBlocked)
{
	if (UYSCharacterMovementComponent* YSMovement = Cast<UYSCharacterMovementComponent>(GetCharacterMovement()))
	{
		YSMovement->SetMovementBlocked(bBlocked);
	}
}

void AYSCharacterPlayer::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	AbilitySystemComponent->GiveInitAbility();
}

void AYSCharacterPlayer::ProcessMovementInput(const FVector2D& InputDir)
{
	TSet<FGameplayTag> NewTags;
	
	if (InputDir.Y > 0.f)  NewTags.Add(YSInputTags::InputUp);
	if (InputDir.Y < 0.f)  NewTags.Add(YSInputTags::InputDown);
	if (InputDir.X > 0.f)  NewTags.Add(YSInputTags::InputRight);
	if (InputDir.X < 0.f)  NewTags.Add(YSInputTags::InputLeft);
	
	// 새로 들어온 태그만 AcceptInput 호출
	for (const FGameplayTag& Tag : NewTags)
	{
		if (!LastDirectionTags.Contains(Tag))
			InputStateMachineComponent->AcceptInput(Tag);
	}
	
	LastDirectionTags = NewTags;
}
