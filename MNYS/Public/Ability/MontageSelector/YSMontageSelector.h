// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "General/YSEnum.h"
#include "StructUtils/InstancedStruct.h"
#include "YSMontageSelector.generated.h"

class UYSGameplayAbility;

// 열거값 = 8섹터 인덱스. atan2(Y, X) 각도를 22.5° 오프셋으로 시계방향 45° 단위로 나눈다 (0 Forward, 4 Backward).

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



USTRUCT(BlueprintType)
struct MNYS_API FYSMontageSelector
{
	GENERATED_BODY()

	virtual ~FYSMontageSelector() = default;
	virtual UAnimMontage* SelectMontage(const UYSGameplayAbility* Ability) const { return nullptr; }

	virtual void SetMotionWarp(const UYSGameplayAbility* Ability, bool bInSet) const;
	
public :
	// 비우면 워핑하지 않는다.
	UPROPERTY(EditAnywhere, meta = (DisplayName = "모션 워핑 파라미터", BaseStruct = "/Script/MNYS.YSMotionWarpParam", ExcludeBaseStruct))
	FInstancedStruct MotionWarpingParam;
};


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


// 로컬 속도 XY 각도를 45° 8섹터로 나눠 몽타주를 고른다.

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

UENUM(BlueprintType)
enum class EYSDirectionSource : uint8
{
	// 이벤트를 보낸 상대(공격자)가 내 로컬 기준 어느 쪽에 있는가. 히트 리액션용.
	Instigator   UMETA(DisplayName = "공격자 기준"),

	// 내가 지금 어느 쪽으로 입력을 넣고 있는가. 회피/스텝용.
	ControlInput UMETA(DisplayName = "입력 기준"),
};


// 조회 태그(이벤트 태그 + 추가 태그 + 옵션 4방향 태그)를 런타임에 조립해 SelectBest 에 넘긴다. 방향은 공격자가 모르는 값이라 피격 시점에 계산해 합성한다.

USTRUCT(BlueprintType, DisplayName = "태그별 몽타주 선택")
struct MNYS_API FYSMontageSelector_ByTag : public FYSMontageSelector
{
	GENERATED_BODY()

public:
	virtual UAnimMontage* SelectMontage(const UYSGameplayAbility* Ability) const override;

	FGameplayTagContainer BuildQueryTags(const UYSGameplayAbility* Ability) const;

public:
	UPROPERTY(EditAnywhere, meta = (DisplayName = "방향 태그 사용"))
	bool bUseDirectionTag = false;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "방향 기준", EditCondition = "bUseDirectionTag", EditConditionHides))
	EYSDirectionSource DirectionSource = EYSDirectionSource::Instigator;

	// 항상 조회 태그에 얹을 태그. 이벤트가 실어주지 않는 상황(공중 피격 등)을 데이터에서 강제할 때 쓴다.
	UPROPERTY(EditAnywhere, meta = (DisplayName = "추가 태그"))
	FGameplayTagContainer ExtraTags;
	
	UPROPERTY(EditAnywhere, meta = (DisplayName = "옵션 태그"))
	FGameplayTagContainer OwnedTagFilter;
};
