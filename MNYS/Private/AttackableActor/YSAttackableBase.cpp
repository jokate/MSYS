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

void AYSAttackableBase::BeginPlay()
{
	Super::BeginPlay();
	
	switch (ActivationType)
	{
	case EYSAttackActivationType::Instant :
		{
			OnActivate();
			break;
		}
	case EYSAttackActivationType::TagBased :
		{
			UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerActor.Get());
			
			if ( IsValid(ASC) )
			{
				ASC->GenericGameplayEventCallbacks.FindOrAdd(EventTag).AddUObject(this, &AYSAttackableBase::OnActivateTagCallback);
			}
		}
	case EYSAttackActivationType::TimeBased :
		{
			if ( ActivateTime > 0.f )
			{
				GetWorldTimerManager().SetTimer(ActivateTimerHandle, this, &AYSAttackableBase::OnActivate, ActivateTime, false);	
			}
		}
	}
}

void AYSAttackableBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	switch (ActivationType)
	{
	case EYSAttackActivationType::TagBased :
		{
			UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerActor.Get());
			
			if ( IsValid(ASC) )
			{
				ASC->GenericGameplayEventCallbacks.Remove(EventTag);
			}
		}
	case EYSAttackActivationType::TimeBased :
		{
			GetWorldTimerManager().ClearTimer(ActivateTimerHandle);	
		}
	default : 
		break;
	}
	Super::EndPlay(EndPlayReason);
}

void AYSAttackableBase::OnActivateTagCallback(const FGameplayEventData* GameplayEventData)
{
	OnActivate();
}
