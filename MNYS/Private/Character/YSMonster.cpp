// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/YSMonster.h"

#include "YSAbilitySystemComponent.h"
#include "AI/YSAIController.h"
#include "General/YSDefine.h"

// Sets default values
AYSMonster::AYSMonster(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AYSMonster::BeginPlay()
{
	Super::BeginPlay();
	SetGenericTeamId(YS_MONSTER_START);
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	AbilitySystemComponent->GiveInitAbility();
}

// Called every frame
void AYSMonster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

