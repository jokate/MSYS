// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "YSAttackableBase.h"
#include "YSAttackSpawner.generated.h"

struct FYSSpawnActorConfig;

UCLASS()
class MNYS_API AYSAttackSpawner : public AYSAttackableBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AYSAttackSpawner();

#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif
	
protected:
	virtual void OnActivate() override;

	void SpawnActorByConfig(FYSSpawnActorConfig SpawnConfig);
	virtual void TrySpawnActor();
	
	
protected : 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn", meta = (DisplayName = "스폰 처리 관련"))
	TArray<FYSSpawnActorConfig> SpawnActorConfigs;

	FTimerHandle SpawnTimerHandle;

	int32 SpawnCount = 0;
};

