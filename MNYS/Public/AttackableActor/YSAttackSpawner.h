// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "YSAttackableBase.h"
#include "YSAttackSpawner.generated.h"

class UArrowComponent;
struct FYSSpawnActorConfig;


/*
 * 스포너의 경우에는 전조 -> 스폰의 형태를 기반으로 스폰처리를 담당합니다.
 * 만약에 즉시 스폰 처리를 하고자 한다면, 해당 기능을 사용하면 안됩니다.
 * 특별한 스폰규칙이 있는경우에는 1 : N의 형태로 스폰이 가능하며 해당 부분에 대한 참고 부탁드립니다.
 */

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
	void SpawnActorByConfig(FYSSpawnActorConfig SpawnConfig);
	virtual void TrySpawnActor();

protected :
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | Spawn", meta = (DisplayName = "스폰 처리 관련"))
	TArray<FYSSpawnActorConfig> SpawnActorConfigs;
	
	int32 SpawnCount = 0;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	TArray<TObjectPtr<UArrowComponent>> SpawnPreviewArrows;

	void _RefreshSpawnPreview();
#endif
};

