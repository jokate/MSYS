// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackableActor/YSAttackableBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"


// Sets default values
AYSAttackableBase::AYSAttackableBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
	
	RootMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RootMesh"));
	RootMesh->SetupAttachment(SceneRoot);

}

void AYSAttackableBase::AllocateInstigator(AActor* InInstigator)
{
	OwnerActor = InInstigator;
}
