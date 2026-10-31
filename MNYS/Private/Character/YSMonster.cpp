// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/YSMonster.h"

#include "YSAbilitySystemComponent.h"
#include "AI/YSAIController.h"
#include "AI/Component/YSPhaseChangeComponent.h"
#include "Character/AttributeSet/YSGroggyAttributeSet.h"
#include "Combat/YSGroggyComponent.h"
#include "General/YSDefine.h"

AYSMonster::AYSMonster(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	
	PhaseChangeComponent = CreateDefaultSubobject<UYSPhaseChangeComponent>(TEXT("PhaseChangeComponent"));
}

void AYSMonster::BeginPlay()
{
	Super::BeginPlay();
	SetGenericTeamId(YS_MONSTER_START);
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	AbilitySystemComponent->ApplyLevelHpScale();
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

void AYSMonster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

