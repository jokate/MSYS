// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackableActor/YSAttackableBase.h"


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

void AYSAttackableBase::BeginPlay()
{
	Super::BeginPlay();
	
	if ( ActivateTime > 0.f )
	{
		GetWorldTimerManager().SetTimer(ActivateTimerHandle, this, &AYSAttackableBase::OnActivate, ActivateTime, false);	
	}
	else
	{
		OnActivate();
	}
}
