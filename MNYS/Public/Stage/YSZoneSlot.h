// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "YSZoneSlot.generated.h"

class UBoxComponent;

/** 스테이지 맵에 놓는 구역 자리. 띄우고 내리는 판단은 게임 모드가 하고, 이 액터는 자리 번호와 크기만 갖는다. */
UCLASS()
class MNYS_API AYSZoneSlot : public AActor
{
	GENERATED_BODY()

public :
	AYSZoneSlot();

	int32 GetSlotIndex() const { return SlotIndex; }

	/** 상자 밖까지의 거리(cm). 안에 있으면 0. */
	float GetDistanceTo(const FVector& Location) const;

private :
	// 스테이지 정의 표의 자리 배열 순번과 맞춘다.
	UPROPERTY(EditInstanceOnly, Category = "YS | Stage", meta = (DisplayName = "자리 번호", ClampMin = "0"))
	int32 SlotIndex = 0;

	// 구역 레벨이 이 상자 안에 들어와야 구역끼리 겹치지 않는다.
	UPROPERTY(VisibleAnywhere, Category = "YS | Stage", meta = (DisplayName = "자리 범위"))
	TObjectPtr<UBoxComponent> Bounds;
};
