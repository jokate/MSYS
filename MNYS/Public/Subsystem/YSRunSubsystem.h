// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "YSRunSubsystem.generated.h"

/** 런 시작 때 정한 스테이지 하나의 배치. 이후 다시 뽑지 않는다. */
USTRUCT()
struct FYSStagePlan
{
	GENERATED_BODY()

	UPROPERTY()
	FName StageRow;

	UPROPERTY()
	TSoftObjectPtr<UWorld> StageMap;

	// 자리 번호 순. 비어 있으면 그 자리에 구역이 없다.
	UPROPERTY()
	TArray<TSoftObjectPtr<UWorld>> SlotZones;
};

/** 구역 하나의 런 중 상태. 처치한 액터는 구역 레벨 안의 액터 이름으로 남긴다. */
USTRUCT()
struct FYSZoneState
{
	GENERATED_BODY()

	UPROPERTY()
	bool bCleared = false;

	UPROPERTY()
	TSet<FName> DefeatedActors;
};

/** 런 배치와 지금 스테이지의 구역 상태를 런 동안 메모리에 둔다. */
UCLASS()
class MNYS_API UYSRunSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public :
	/** 스테이지 정의 표에서 스테이지와 자리마다 구역을 뽑고 첫 스테이지로 간다. */
	UFUNCTION(BlueprintCallable, Category = "YS | Run")
	bool StartRun();

	/** 다음 스테이지로 간다. 지난 스테이지의 구역 상태는 버린다. 마지막 스테이지였으면 false. */
	UFUNCTION(BlueprintCallable, Category = "YS | Run")
	bool AdvanceStage();

	/** 지금 스테이지에서 이 자리에 띄울 구역. 런이 없거나 World 가 지금 스테이지 맵이 아니면 비어 있다. */
	TSoftObjectPtr<UWorld> GetZoneLevel(const UWorld* World, int32 SlotIndex) const;

	const FYSZoneState* FindZoneState(int32 SlotIndex) const { return ZoneStates.Find(SlotIndex); }

	void RecordDefeated(int32 SlotIndex, FName ActorName);

	UFUNCTION(BlueprintCallable, Category = "YS | Run")
	void MarkZoneCleared(int32 SlotIndex);

private :
	void OpenCurrentStage() const;

	UPROPERTY()
	TArray<FYSStagePlan> StagePlans;

	UPROPERTY()
	int32 CurrentStageIndex = INDEX_NONE;

	UPROPERTY()
	TMap<int32, FYSZoneState> ZoneStates;
};
