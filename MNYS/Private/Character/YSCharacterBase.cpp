// Fill out your copyright notice in the Description page of Project Settings.


#include "MNYS/Public/Character/YSCharacterBase.h"
#include "YSAbilitySystemComponent.h"
#include "Character/AttributeSet/YSCharacterAttributeSetBase.h"
#include "Character/Components/YSCharacterMovementComponent.h"
#include "Framework/YSGameModeBase.h"
#include "GameFramework/SpringArmComponent.h"
#include "General/YSDefine.h"
#include "MotionWarp/UYSMotionWarpingComponent.h"


AYSCharacterBase::AYSCharacterBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UYSCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
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

float AYSCharacterBase::GetHpRatio() const
{
	// IsDead 와 같은 방향(죽은 쪽)으로 실패한다. 호출자는 위상 전환 HP 조건(<=)과 AI 점수 입력이다.
	if (IsValid(AbilitySystemComponent) == false)
		return 0.f;

	const UYSCharacterAttributeSetBase* AttributeSet = AbilitySystemComponent->GetSet<UYSCharacterAttributeSetBase>();
	if (AttributeSet == nullptr)
		return 0.f;

	if (AttributeSet->GetMaxHp() <= 0.f)
		return 0.f;

	return AttributeSet->GetHp() / AttributeSet->GetMaxHp();
}

void AYSCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

UYSCharacterMovementComponent* AYSCharacterBase::GetYSCharacterMovement()
{
	return Cast<UYSCharacterMovementComponent>(GetCharacterMovement());
}

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

const FYSCharacterInfo* AYSCharacterBase::GetCharacterInfo() const
{
	if ( CharacterInfo.IsNull() )
	{
		return nullptr;
	}
	
	return CharacterInfo.GetRow<FYSCharacterInfo>(TEXT("context"));
}

