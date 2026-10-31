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
AYSCharacterPlayer::AYSCharacterPlayer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UYSCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	
	PrimaryActorTick.bCanEverTick = true;
	
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 600.0f;
	CameraBoom->bUsePawnControlRotation = true;

	
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	InputStateMachineComponent = CreateDefaultSubobject<UYSInputStateMachineComponent>(TEXT("InputStateMachine"));

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f);
	
}

void AYSCharacterPlayer::BeginPlay()
{
	Super::BeginPlay();

	SetGenericTeamId(YS_PLAYER);
}

void AYSCharacterPlayer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AYSCharacterPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

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

				// 홀드 바인딩은 없다. 시간 기반 트리거가 없는 Boolean IA 는 Ongoing 을 거치지 않으므로, "지금 눌려 있는가"는 Pressed/Released 로부터 UYSInputStateMachineComponent 가 유도한다.
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

void AYSCharacterPlayer::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if ( Controller == nullptr )
	{
		return;
	}
	const UYSCameraManageComponent* CameraManager = UYSCameraManageComponent::Get(this);

	// 요가 잠기면 마우스 X 는 커서 몫이다. 여기서 흘려보내면 캐릭터와 스프링암이 같이 돌아 조준이 미끄러진다.
	if ( IsValid(CameraManager) == false || CameraManager->IsControlYawLocked() == false )
	{
		AddControllerYawInput(LookAxisVector.X);
	}

	if ( IsValid(CameraManager) == false || CameraManager->IsControlPitchLocked() == false )
	{
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AYSCharacterPlayer::Move(const FInputActionValue& Value)
{
	if ( IsValid( GetYSCharacterMovement() ) == false )
	{
		return;
	}

	if ( GetYSCharacterMovement()->IsMovementBlocked() )
		return;
	
	FVector2D MovementVector = Value.Get<FVector2D>();

	ProcessMovementInput(MovementVector.GetSafeNormal());
	
	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
			
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AYSCharacterPlayer::OnMovementComplete()
{
	for (const FGameplayTag& Tag : LastDirectionTags)
	{
		InputStateMachineComponent->ClearInputHeld(Tag);
	}

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

	// 어빌리티 부여보다 스탯 초기화가 먼저다. 쿨다운/배율이 어트리뷰트를 참조하므로 기본값 상태에서 부여되면 첫 산출이 어긋난다.
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
	
	// 방향 태그는 축 입력에서 합성한 것이라 "뗌"이 없다. 진입 순간을 누름으로 취급한다.
	for (const FGameplayTag& Tag : NewTags)
	{
		if (!LastDirectionTags.Contains(Tag))
			InputStateMachineComponent->AcceptInput(Tag, EYSInputPhase::Pressed);
	}

	for (const FGameplayTag& Tag : LastDirectionTags)
	{
		if (!NewTags.Contains(Tag))
			InputStateMachineComponent->ClearInputHeld(Tag);
	}

	LastDirectionTags = NewTags;
}
