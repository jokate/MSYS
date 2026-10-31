// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "YSAttackableBase.h"
#include "YSAttackSpawner.generated.h"

class UArrowComponent;
struct FYSSpawnActorConfig;


/* 전조 → 스폰 형태의 스폰을 맡는다. 즉시 스폰이 필요하면 쓰지 말 것. 특별한 스폰 규칙이 있으면 1 : N 스폰이 가능하다. */

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
	virtual void OnActivate_Implementation() override;
	virtual void TrySpawnActor();

protected :
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | Spawn", meta = (DisplayName = "스폰 처리 관련"))
	TArray<FYSSpawnActorConfig> SpawnActorConfigs;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	TArray<TObjectPtr<UArrowComponent>> SpawnPreviewArrows;

	void _RefreshSpawnPreview();
#endif
};

