// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "YSSwitchWindowCloseCondition.generated.h"

class AActor;
class APawn;
class APlayerController;

/** 열린 스위치 창을 조기 닫기 조건에 넘기는 값. 스쿼드가 매 틱 채운다. */
struct MNYS_API FYSSwitchWindowCloseContext
{
	const APlayerController* Controller = nullptr;

	// 창 GE 가 걸린 캐릭터
	const APawn* Pawn = nullptr;

	// 창을 열 때의 대상
	const AActor* Target = nullptr;

	// 창이 열린 뒤 지난 시간 (월드 시계)
	float OpenedFor = 0.f;
};

/**
 * 스위치 창 조기 닫기 조건. 상한은 창 GE 지속이고, 이 조건은 그보다 먼저 닫을 때만 쓴다.
 * 트리거마다 YS Developer Settings 에서 고른다. 새 조건이 오면 파생 구조체 하나를 더한다.
 */
USTRUCT()
struct MNYS_API FYSSwitchWindowCloseCondition
{
	GENERATED_BODY()
	virtual ~FYSSwitchWindowCloseCondition() = default;

	/** 최소 창 길이가 지났고 조건이 참이면 닫는다. */
	bool CanCloseEarly(const FYSSwitchWindowCloseContext& Context) const;

	// 이 시간 전에는 조건이 참이어도 닫지 않는다. 작은 전환에서 다음 틱에 닫혀 입력할 틈이 없어지는 것을 막는다.
	UPROPERTY(EditAnywhere, Category = "YS | Switch", meta = (DisplayName = "최소 창 길이 (초)", ClampMin = "0"))
	float MinOpenTime = 0.15f;

protected:
	virtual bool ShouldClose(const FYSSwitchWindowCloseContext& Context) const { return false; }
};

/** 전환 회전이 끝나면 닫는다. 캐릭터는 즉시 돌고 카메라(컨트롤 요)가 보간으로 따라오므로 컨트롤 요로 본다. */
USTRUCT(DisplayName = "카메라 요가 대상을 향하면")
struct MNYS_API FYSSwitchWindowCloseCondition_ControlYawAligned : public FYSSwitchWindowCloseCondition
{
	GENERATED_BODY()

	// 컨트롤 요와 캐릭터→대상 방향의 요 차이가 이 안이면 회전이 끝난 것으로 본다. 피치는 보지 않는다.
	UPROPERTY(EditAnywhere, Category = "YS | Switch", meta = (DisplayName = "허용 각 (도)", ClampMin = "0", ClampMax = "180"))
	float YawTolerance = 10.f;

protected:
	virtual bool ShouldClose(const FYSSwitchWindowCloseContext& Context) const override;
};
