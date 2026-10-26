// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/YSMonster.h"

#include "YSAbilitySystemComponent.h"
#include "AI/YSAIController.h"
#include "AI/Component/YSPhaseChangeComponent.h"
#include "Character/AttributeSet/YSGroggyAttributeSet.h"
#include "Combat/YSGroggyComponent.h"
#include "General/YSDefine.h"

// Sets default values
AYSMonster::AYSMonster(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	PhaseChangeComponent = CreateDefaultSubobject<UYSPhaseChangeComponent>(TEXT("PhaseChangeComponent"));
}

// Called when the game starts or when spawned
void AYSMonster::BeginPlay()
{
	Super::BeginPlay();
	SetGenericTeamId(YS_MONSTER_START);
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	AbilitySystemComponent->GiveInitAbility();
}

void AYSMonster::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if ( StaggerGrade == EYSStaggerGrade::Immovable )
	{
		AbilitySystemComponent->AddSet<UYSGroggyAttributeSet>();

		GroggyComponent = NewObject<UYSGroggyComponent>(this, TEXT("GroggyComponent"));
		GroggyComponent->SetProfile(GroggyProfile);
		GroggyComponent->RegisterComponent();
	}
}

// Called every frame
void AYSMonster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

