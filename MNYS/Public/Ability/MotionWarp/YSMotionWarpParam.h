// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MotionWarpingComponent.h"
#include "YSMotionWarpParam.generated.h"

class UYSGameplayAbility;
/**
 * 모션 워핑에 대한 파라미터를 담는 클래스입니다. 모션 워핑의 경우, 단순히 타겟 위치로 이동하는 것 이상의 다양한 정책이 나올 수 있기 때문에, 해당 클래스를 상속받아서 필요한 데이터를 담도록 합니다.
 */
USTRUCT(BlueprintType)
struct MNYS_API FYSMotionWarpParam 
{
	GENERATED_BODY()
	
	virtual ~FYSMotionWarpParam() = default;

	virtual FMotionWarpingTarget GetWarpTargetData(const UYSGameplayAbility* InAbility) const { return FMotionWarpingTarget();  }
	virtual void ResolveMotionWarp(const UYSGameplayAbility* InAbility) const;
	virtual void ReleaseMotionWarp(const UYSGameplayAbility* InAbility) const;
	
public : 
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName MotionWarpTargetName = NAME_None;
};


USTRUCT(BlueprintType, DisplayName = "컨트롤 로테이션 기준 모션 워핑")
struct MNYS_API FYSMotionWarpParam_ControlRotation : public FYSMotionWarpParam
{
	GENERATED_BODY()
	
	virtual FMotionWarpingTarget GetWarpTargetData(const UYSGameplayAbility* InAbility) const override;
};

USTRUCT(BlueprintType, DisplayName = "히트 리액션용 모션 워핑 ( 타겟을 향하도록 )")
struct MNYS_API FYSMotionWarpParam_HitReaction : public FYSMotionWarpParam
{
	GENERATED_BODY()

	virtual FMotionWarpingTarget GetWarpTargetData(const UYSGameplayAbility* InAbility) const override;
};
