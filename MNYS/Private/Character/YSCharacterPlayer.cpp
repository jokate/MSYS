// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/YSCharacterPlayer.h"

#include "EnhancedInputSubsystems.h"
#include "YSAbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Character/AttributeSet/YSCharacterAttributeSetBase.h"
#include "Character/Components/YSCameraManageComponent.h"
#include "Character/Components/YSCharacterMovementComponent.h"
#include "Character/Components/YSTargetingComponent.h"
#include "Framework/YSGameModeBase.h"
#include "GameFramework/SpringArmComponent.h"
#include "General/YSDefine.h"
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
	bUseControllerRotationYaw = false;
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

void AYSCharacterPlayer::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);
	
	if (GetCharacterMovement()->IsFalling())
	{
		AddStateToStateMachine(EYSInputStatesType::Falling);
	}
	else
	{
		RemoveStateToStateMachine(EYSInputStatesType::Falling);
	}
}


// Called when the game starts or when spawned
void AYSCharacterPlayer::BeginPlay()
{
	Super::BeginPlay();

	// 팀설정.
	SetGenericTeamId(YS_PLAYER);
}

void AYSCharacterPlayer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
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
				EnhancedInputComponent->BindActionByTag(InputConfig, InputAction.InputTag, ETriggerEvent::Started, this,
					&AYSCharacterPlayer::ProcessInput, InputAction.InputTag, EYSInputPhase::Pressed);

				EnhancedInputComponent->BindActionByTag(InputConfig, InputAction.InputTag, ETriggerEvent::Completed, this,
					&AYSCharacterPlayer::ProcessInput, InputAction.InputTag, EYSInputPhase::Released);
				
				EnhancedInputComponent->BindActionByTag(InputConfig, InputAction.InputTag, ETriggerEvent::Canceled, this,
					&AYSCharacterPlayer::ProcessInput, InputAction.InputTag, EYSInputPhase::Canceled);

				// 홀드 바인딩은 없다. ETriggerEvent::Ongoing은 ETriggerState(Ongoing -> Ongoing) 전이에서만
				// 나오는데, 시간 기반 트리거가 없는 Boolean IA는 None -> Triggered로 직행해 그 상태를 거치지 않는다.
				// "지금 눌려 있는가"는 위의 Pressed/Released로부터 UYSInputStateMachineComponent가 유도한다.
			}
		}
	}
}

void AYSCharacterPlayer::ProcessInput(FGameplayTag InputTag, EYSInputPhase InputPhase)
{
	if ( IsValid(InputStateMachineComponent))
	{
		InputStateMachineComponent->AcceptInput(InputTag, InputPhase);
	}
}

void AYSCharacterPlayer::AddStateToStateMachine(EYSInputStatesType InputStateType) const
{
	if ( IsValid(InputStateMachineComponent) )
	{
		InputStateMachineComponent->AddStateStack(InputStateType);
	}
}

void AYSCharacterPlayer::RemoveStateToStateMachine(EYSInputStatesType InputStateType) const
{
	if ( IsValid(InputStateMachineComponent) )	
	{
		InputStateMachineComponent->RemoveStateStack(InputStateType);
	}
}

void AYSCharacterPlayer::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if ( Controller == nullptr )
	{
		return;
	}
	UYSTargetingComponent* Targeting = UYSTargetingComponent::Get(this);
	
	if ( IsValid(Targeting) && Targeting->IsCursorMode() )
	{
		return;
	}
	
	
	AddControllerYawInput(LookAxisVector.X);
	
	const UYSCameraManageComponent* CameraManager = UYSCameraManageComponent::Get(this);

	if ( IsValid(CameraManager) && CameraManager->IsControlPitchLocked() )
	{
		return;
	}

	AddControllerPitchInput(LookAxisVector.Y);
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

	// 어빌리티 부여보다 스탯 초기화가 먼저다.
	// 어빌리티 쿨다운/배율이 어트리뷰트를 참조하므로, 기본값 상태에서 부여되면 첫 산출이 어긋난다.
	AbilitySystemComponent->ApplyStatInitialization();
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
	// 방향 태그는 축 입력에서 합성한 것이라 "뗌"이 없다. 진입 순간을 누름으로 취급한다.
	for (const FGameplayTag& Tag : NewTags)
	{
		if (!LastDirectionTags.Contains(Tag))
			InputStateMachineComponent->AcceptInput(Tag, EYSInputPhase::Pressed);
	}
	
	LastDirectionTags = NewTags;
}
