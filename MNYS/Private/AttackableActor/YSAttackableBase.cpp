// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackableActor/YSAttackableBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AttackableActor/YSTelegraphActor.h"


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
	ProcessActivationType();
}

void AYSAttackableBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	HitContext = nullptr;
	DeprocessActivationType();
	Super::EndPlay(EndPlayReason);
}

void AYSAttackableBase::OnActivate_Implementation()
{
	if ( IsValid(TelegraphActor) ) 
	{
		TelegraphActor->Destroy();
	}
	
	if ( DestroyDelay > 0.f )
	{
		GetWorldTimerManager().SetTimer(DestroyTimerHandle, this, &AYSAttackableBase::DestroyActor, DestroyDelay, false);	
	}
}

void AYSAttackableBase::ProcessActivationType()
{
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
			break;
		}
	case EYSAttackActivationType::TimeBased :
		{
			if ( ActivateTime > 0.f )
			{
				GetWorldTimerManager().SetTimer(ActivateTimerHandle, this, &AYSAttackableBase::OnActivate, ActivateTime, false);	
			}
			break;
		}
	}
	
	ProcessTelegraph();
}

void AYSAttackableBase::DeprocessActivationType()
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
}

void AYSAttackableBase::OnActivateTagCallback(const FGameplayEventData* GameplayEventData)
{	
	//2026.06.15 일단은 태그 이벤트가 들어왔을 때, 공격이 활성화 되는 형태로 만들어 놓긴 했는데,
	//추후에 태그 이벤트가 들어왔을 때마다 공격이 활성화 되는 형태로 만들 수도 있을 것 같긴 함 ( 그럴 경우에는 ActivateTimeBased 같은 형태로 만들어야 할 듯 )
	OnActivate();
}

void AYSAttackableBase::DestroyActor()
{
	Destroy();
}
