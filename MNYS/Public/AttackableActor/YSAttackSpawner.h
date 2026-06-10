// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "YSAttackableBase.h"
#include "YSAttackSpawner.generated.h"

class UArrowComponent;
struct FYSSpawnActorConfig;

UCLASS()
class MNYS_API AYSAttackSpawner : public AYSAttackableBase
{
	GENERATED_BODY()

public:
	AYSAttackSpawner();

#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
	virtual void OnConstruction(const FTransform& Transform) override;
#endif

protected:
	virtual void OnActivate() override;

	void SpawnActorByConfig(FYSSpawnActorConfig SpawnConfig);
	virtual void TrySpawnActor();

protected :
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | Spawn", meta = (DisplayName = "스폰 처리 관련"))
	TArray<FYSSpawnActorConfig> SpawnActorConfigs;

	FTimerHandle SpawnTimerHandle;

	int32 SpawnCount = 0;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	TArray<TObjectPtr<UArrowComponent>> SpawnPreviewArrows;

	void _RefreshSpawnPreview();
#endif
};

