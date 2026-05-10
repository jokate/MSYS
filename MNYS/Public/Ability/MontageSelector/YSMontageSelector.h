// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Ability/YSGameplayAbility.h"
#include "General/YSEnum.h"
#include "YSMontageSelector.generated.h"

class UYSGameplayAbility;

// ─── 8방향 열거형 ─────────────────────────────────────────────────────────────
//
//  atan2(LocalVelocity.Y, LocalVelocity.X) 각도 기준으로 22.5° 간격 섹터에 매핑.
//  열거값 순서 = 섹터 인덱스 (0°에서 시계방향으로 45° 단위).
//
//       0 Forward (0°)
//  7 FL          1 FR
//  6 Left        2 Right
//  5 BL          3 BR
//       4 Backward (±180°)

UENUM(BlueprintType)
enum class EYSMoveDirection : uint8
{
	Forward      = 0,
	ForwardRight = 1,
	Right        = 2,
	BackwardRight= 3,
	Backward     = 4,
	BackwardLeft = 5,
	Left         = 6,
	ForwardLeft  = 7,
};


// ─── Base ────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct MNYS_API FYSMontageSelector
{
	GENERATED_BODY()

	virtual ~FYSMontageSelector() = default;
	virtual UAnimMontage* SelectMontage(const UYSGameplayAbility* Ability) const { return nullptr; }

	virtual void SetMotionWarp(const UYSGameplayAbility* Ability, bool bInSet) const;
	
public :
	// 차후 모션워핑 정책이 달라질 수 있음, 달라지면 이것도 InstancedStruct로 개설하도록 합니다
	UPROPERTY(EditAnywhere, meta = (DisplayName = "모션 워핑 사용"))
	bool bUseMotionWarping = false;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "모션 워핑 타입", EditCondition = bUseMotionWarping, EditConditionHides))
	EMotionWarpType MotionWarpType = EMotionWarpType::None;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "모션 워핑 타겟 ID", EditCondition = bUseMotionWarping, EditConditionHides))
	FName MotionWarpName = NAME_None;
};


// ─── 단일 몽타주 ─────────────────────────────────────────────────────────────
USTRUCT(BlueprintType, DisplayName = "단일 몽타주 선택")
struct MNYS_API FYSMontageSelector_Default : public FYSMontageSelector
{
	GENERATED_BODY()

public:
	virtual UAnimMontage* SelectMontage(const UYSGameplayAbility* Ability) const override
	{
		return DefaultMontage.LoadSynchronous();
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName = "재생할 몽타주")
	TSoftObjectPtr<UAnimMontage> DefaultMontage;
};


// ─── 이동 방향 기반 (8방향) ───────────────────────────────────────────────────
//
//  로컬 속도 XY 평면의 각도를 45° 단위 8섹터로 분할하여 몽타주를 선택한다.
//  섹터 경계는 22.5° 오프셋으로 각 방향 중심에 정렬된다.
//  등록되지 않은 방향은 Forward로 폴백.

USTRUCT(BlueprintType, DisplayName = "방향별 몽타주 선택 (8방향)")
struct MNYS_API FYSMontageSelector_ByDirection : public FYSMontageSelector
{
	GENERATED_BODY()

public:
	virtual UAnimMontage* SelectMontage(const UYSGameplayAbility* Ability) const override;

public:
	// 방향별 몽타주. 비어있는 방향은 Forward 로 폴백.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName = "방향별 몽타주")
	TMap<EYSMoveDirection, TSoftObjectPtr<UAnimMontage>> DirectionMontages;
};
