// Fill out your copyright notice in the Description page of Project Settings.


#include "MNYS/Public/Character/YSCharacterBase.h"
#include "YSAbilitySystemComponent.h"
#include "Character/AttributeSet/YSCharacterAttributeSetBase.h"
#include "Character/Components/YSCharacterMovementComponent.h"
#include "Framework/YSGameModeBase.h"
#include "GameFramework/SpringArmComponent.h"
#include "General/YSDefine.h"
#include "MotionWarp/UYSMotionWarpingComponent.h"


// Sets default values
AYSCharacterBase::AYSCharacterBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UYSCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	AbilitySystemComponent = CreateDefaultSubobject<UYSAbilitySystemComponent>(TEXT("ASC"));
	MotionWarpingComponent = CreateDefaultSubobject<UYSMotionWarpingComponent>(TEXT("MotionWarping"));
	WeaponMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	if ( IsValid(WeaponMeshComponent) )
	{
		WeaponMeshComponent->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, TEXT("Weapon"));
	}
	
}

// Called when the game starts or when spawned
void AYSCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
	AYSGameModeBase* GM = GetWorld()->GetAuthGameMode<AYSGameModeBase>();
	
	if ( IsValid(GM) )
	{
		GM->RegisterTimeDilationActor(this);
	}
}

void AYSCharacterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	AYSGameModeBase* GM = GetWorld()->GetAuthGameMode<AYSGameModeBase>();
	
	if ( IsValid(GM) )
	{
		GM->UnregisterTimeDilationActor(this);
	}
	
	Super::EndPlay(EndPlayReason);
}

void AYSCharacterBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	AbilitySystemComponent->AddSet<UYSCharacterAttributeSetBase>();
}

bool AYSCharacterBase::IsDead() const
{
	if (IsValid(AbilitySystemComponent) == false)
		return true;

	const UYSCharacterAttributeSetBase* AttributeSet = AbilitySystemComponent->GetSet<UYSCharacterAttributeSetBase>();
	if (AttributeSet == nullptr)
		return true;

	return AttributeSet->GetHp() <= 0.f;
}

// Called every frame
void AYSCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

UYSCharacterMovementComponent* AYSCharacterBase::GetYSCharacterMovement()
{
	return Cast<UYSCharacterMovementComponent>(GetCharacterMovement());
}

// Called to bind functionality to input
void AYSCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

UAbilitySystemComponent* AYSCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AYSCharacterBase::SetGenericTeamId(const FGenericTeamId& TeamID)
{
	TeamID.SetAttitudeSolver(YSTeamFunction::GeneralTeamSolver);
	GenericTeamId = TeamID;
}

