// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/GameplayCueActor/YSGameplayCueNotify_ChangeMaterial.h"


// Sets default values
AYSGameplayCueNotify_ChangeMaterial::AYSGameplayCueNotify_ChangeMaterial()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AYSGameplayCueNotify_ChangeMaterial::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AYSGameplayCueNotify_ChangeMaterial::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

