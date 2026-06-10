// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackableActor/YSAttackSpawner.h"

#include "General/YSStruct.h"
#include "Library/YSBlueprintFunctionLibrary.h"


// Sets default values
AYSAttackSpawner::AYSAttackSpawner()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

void AYSAttackSpawner::OnActivate()
{
	Super::OnActivate();
	
	TrySpawnActor();
}

void AYSAttackSpawner::SpawnActorByConfig(FYSSpawnActorConfig SpawnConfig)
{
	++SpawnCount;
	
	const FVector Position = UYSBlueprintFunctionLibrary::GetEventPosition(SpawnConfig.PositionPolicy, OwnerActor.Get(), SpawnConfig.SpawnSocket, SpawnConfig.RelativeOffset);
	const FRotator Rotation = UYSBlueprintFunctionLibrary::GetEventRotation(SpawnConfig.RotationPolicy, OwnerActor.Get(), SpawnConfig.RotationSocket, TargetActor.Get());
	
	FTransform SpawnTransform = FTransform(Rotation, Position);
	AActor* SpawnedActor = GetWorld()->SpawnActorDeferred<AActor>(SpawnConfig.ActorClass, SpawnTransform);

	if (IsValid(SpawnedActor) == false)
	{
		return;
	}

	AYSAttackableBase* AttackableActor = Cast<AYSAttackableBase>(SpawnedActor);
	if (IsValid(AttackableActor))
	{
		AttackableActor->AllocateInstigator(Owner.Get());
	}

	if (SpawnConfig.bAttachToActor)
	{
		SpawnedActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
	}

	SpawnedActor->FinishSpawning(SpawnTransform);
	
	if ( SpawnCount >= SpawnActorConfigs.Num() )
	{
		Destroy();
	}
}

void AYSAttackSpawner::TrySpawnActor()
{
	for ( const FYSSpawnActorConfig& SpawnConfig : SpawnActorConfigs )
	{
		if (SpawnConfig.SpawnDelay > 0.f )
		{
			GetWorldTimerManager().SetTimer(SpawnTimerHandle, FTimerDelegate::CreateUObject(this, &AYSAttackSpawner::SpawnActorByConfig, SpawnConfig), SpawnConfig.SpawnDelay, false);			
		}
		else
		{
			SpawnActorByConfig(SpawnConfig);
		}
	}
}

#if WITH_EDITOR
void AYSAttackSpawner::PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
	
	if ( PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(AYSAttackSpawner, SpawnActorConfigs ))
	{
		SpawnActorConfigs.StableSort([](const FYSSpawnActorConfig& A, const FYSSpawnActorConfig& B)
		{
			return A.SpawnDelay < B.SpawnDelay;
		});
	}
}
#endif

