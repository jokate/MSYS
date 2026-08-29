// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystem/YSObjectPoolingSubsystem.h"

#include "Interface/YSSpawnInitializable.h"

UYSObjectPoolingSubsystem* UYSObjectPoolingSubsystem::Get(UWorld* World)
{
	if ( IsValid(World) == false )
	{
		return nullptr;
	}

	return World->GetSubsystem<UYSObjectPoolingSubsystem>();
}

void UYSObjectPoolingSubsystem::Deinitialize()
{
	// 서브시스템 제거 될 때는 빠르게 놓아줍시다
	PooledActors.Empty();
	Super::Deinitialize();
}

void UYSObjectPoolingSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	AccumulatedTime += DeltaTime;
	
	if ( AccumulatedTime >= RefreshTime )
	{
		AccumulatedTime = 0.f;
		
		RemoveInActivePoolActors();
	}
}

AActor* UYSObjectPoolingSubsystem::GetPooledActor(TSubclassOf<AActor> ActorClass)
{
	if (PooledActors.Contains(ActorClass) == false)
	{
		return nullptr;
	}

	TArray<TWeakObjectPtr<AActor>>& ActorPool = PooledActors[ActorClass];
	for (int32 i = ActorPool.Num() - 1; i >= 0; --i)
	{
		AActor* PooledActor = ActorPool[i].Get();
		
		if ( IsValid(PooledActor) == false )
		{
			ActorPool.RemoveAt(i);
			continue;
		}
		
		if ( IYSSpawnInitializable* Initializable = Cast<IYSSpawnInitializable>(PooledActor) )
		{
			if ( Initializable->IsPoolActive() )
			{
				continue;
			}
		
			PooledActor->SetActorHiddenInGame(false);
			PooledActor->SetActorEnableCollision(true);
			PooledActor->SetActorTickEnabled(true);
			return PooledActor;
		}
	}

	return nullptr;
}

void UYSObjectPoolingSubsystem::ReturnPooledActor(AActor* Actor)
{
	if ( IsValid(Actor) == false )
	{
		return;
	}

	Actor->SetActorHiddenInGame(true);
	Actor->SetActorEnableCollision(false);
	Actor->SetActorTickEnabled(false);
	Actor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	if ( IYSSpawnInitializable* Initializable = Cast<IYSSpawnInitializable>(Actor) )
	{
		Initializable->SetPoolActive(false);
	}
}

void UYSObjectPoolingSubsystem::AddPooledActor(AActor* Actor)
{
	if ( IsValid(Actor) == false )
	{
		return;
	}

	TSubclassOf<AActor> ActorClass = Actor->GetClass();
	if ( PooledActors.Contains(ActorClass) == false )
	{
		PooledActors.Add(ActorClass, TArray<TWeakObjectPtr<AActor>>());
	}
	
	PooledActors[ActorClass].AddUnique(Actor);
}

void UYSObjectPoolingSubsystem::RemoveInActivePoolActors()
{
	for ( auto It = PooledActors.CreateIterator(); It; ++It )
	{
		TArray<TWeakObjectPtr<AActor>>& ActorPool = It.Value();
			
		for ( int32 i = ActorPool.Num() - 1; i >= 0; --i )
		{
			AActor* PooledActor = ActorPool[i].Get();
				
			if ( IsValid(PooledActor) == false )
			{
				ActorPool.RemoveAt(i);
				continue;
			}
				
			if ( IYSSpawnInitializable* Initializable = Cast<IYSSpawnInitializable>(PooledActor) )
			{
				if ( Initializable->IsPoolActive() == false )
				{
					PooledActor->Destroy();
					ActorPool.RemoveAt(i);
				}
			}
		}
			
		if ( ActorPool.Num() == 0 )
		{
			It.RemoveCurrent();
		}
	}
}

