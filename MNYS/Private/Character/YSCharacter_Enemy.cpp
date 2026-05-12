// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/YSCharacter_Enemy.h"

#include "General/YSDefine.h"

// Sets default values
AYSCharacter_Enemy::AYSCharacter_Enemy(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AYSCharacter_Enemy::BeginPlay()
{
	Super::BeginPlay();
	SetGenericTeamId(YS_MONSTER_START);
}

// Called every frame
void AYSCharacter_Enemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

