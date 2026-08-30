// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "General/YSEnum.h"
#include "StructUtils/InstancedStruct.h"
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

	UPROPERTY(EditAnywhere, meta = (DisplayName = "모션 워핑 파라미터", EditCondition = bUseMotionWarping, EditConditionHides,  BaseStruct = "/Script/MNYS.YSMotionWarpParam", ExcludeBaseStruct))
	FInstancedStruct MotionWarpingParam;
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

USTRUCT(NotBlueprintType)
struct MNYS_API FYSMontageSelector_ByDirection : public FYSMontageSelector
{
	GENERATED_BODY()

public:
	virtual UAnimMontage* SelectMontage(const UYSGameplayAbility* Ability) const override;

	virtual EYSMoveDirection GetMontageDirection(const UYSGameplayAbility* Ability) const { return EYSMoveDirection::Forward; }
public:
	// 방향별 몽타주. 비어있는 방향은 Forward 로 폴백.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName = "방향별 몽타주")
	TMap<EYSMoveDirection, TSoftObjectPtr<UAnimMontage>> DirectionMontages;
};


USTRUCT(BlueprintType, DisplayName = "입력 방향 기준 몽타주 선택")
struct MNYS_API FYSMontageSelector_ControlInputDirection : public FYSMontageSelector_ByDirection
{
	GENERATED_BODY()
	
public :
	virtual EYSMoveDirection GetMontageDirection(const UYSGameplayAbility* Ability) const override;
};

USTRUCT(BlueprintType, DisplayName = "타겟을 향하는 방향 기준 몽타주 선택")
struct MNYS_API FYSMontageSelector_Target : public FYSMontageSelector_ByDirection
{
	GENERATED_BODY()
	
public :
	virtual EYSMoveDirection GetMontageDirection(const UYSGameplayAbility* Ability) const override;
};

// ─── 방향 태그 산출 기준 ──────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EYSDirectionSource : uint8
{
	// 이벤트를 보낸 상대(공격자)가 내 로컬 기준 어느 쪽에 있는가. 히트 리액션용.
	Instigator   UMETA(DisplayName = "공격자 기준"),

	// 내가 지금 어느 쪽으로 입력을 넣고 있는가. 회피/스텝용.
	ControlInput UMETA(DisplayName = "입력 기준"),
};


// ─── 태그별 몽타주 셀렉터 ─────────────────────────────────────────────────────
//
//  조회 태그를 런타임에 조립해서 UYSTaggedMontageAsset::SelectBest 에 넘긴다.
//  조회 태그 = 이벤트 태그(Hit.Big 등) + 추가 태그 + (옵션) 4방향 태그.
//
//  방향은 공격자가 실어보낼 수 없는 값이다 — 공격자는 피격자가 어딜 보는지 모른다.
//  그래서 저작하는 태그가 아니라 피격 시점에 계산해 합성하는 태그로 다룬다.

USTRUCT(BlueprintType, DisplayName = "태그별 몽타주 선택")
struct MNYS_API FYSMontageSelector_ByTag : public FYSMontageSelector
{
	GENERATED_BODY()

public:
	virtual UAnimMontage* SelectMontage(const UYSGameplayAbility* Ability) const override;

	// 이번 재생에 쓸 조회 태그를 조립한다.
	FGameplayTagContainer BuildQueryTags(const UYSGameplayAbility* Ability) const;

public:
	// 조회 태그에 방향을 얹을지
	UPROPERTY(EditAnywhere, meta = (DisplayName = "방향 태그 사용"))
	bool bUseDirectionTag = false;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "방향 기준", EditCondition = "bUseDirectionTag", EditConditionHides))
	EYSDirectionSource DirectionSource = EYSDirectionSource::Instigator;

	// 항상 조회 태그에 얹을 태그. 이벤트가 실어주지 않는 상황(공중 피격 등)을 데이터에서 강제할 때 쓴다.
	UPROPERTY(EditAnywhere, meta = (DisplayName = "추가 태그"))
	FGameplayTagContainer ExtraTags;
};
