// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MotionWarpingComponent.h"
#include "YSMotionWarpParam.generated.h"

class UYSGameplayAbility;
/** 모션 워핑 파라미터. 타겟 이동 이상의 정책이 나올 수 있어 상속해서 필요한 데이터를 담는다. */
USTRUCT(BlueprintType)
struct MNYS_API FYSMotionWarpParam 
{
	GENERATED_BODY()
	
	virtual ~FYSMotionWarpParam() = default;

	/** 걸 워프 타깃을 만든다. 만들 수 없으면(대상 없음 등) false — 0 위치 · 0 회전 타깃을 걸지 않는다. */
	virtual bool GetWarpTargetData(const UYSGameplayAbility* InAbility, FMotionWarpingTarget& OutTarget) const { return false; }
	virtual void ResolveMotionWarp(const UYSGameplayAbility* InAbility) const;
	virtual void ReleaseMotionWarp(const UYSGameplayAbility* InAbility) const;

	/** 이 파라미터를 받는 몽타주의 워프 구간이 위치 이동까지 써야 하나. */
	virtual bool RequiresTranslation() const { return false; }

protected :
	/** 재생 중인 몽타주가 이 워프를 받을 수 없으면(루트 모션 꺼짐 · 같은 이름 구간 없음 · 이동 꺼짐) 몽타주마다 한 번 경고한다. */
	void WarnIfMontageCannotWarp(const UYSGameplayAbility* InAbility) const;
	
public : 
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName MotionWarpTargetName = NAME_None;
};


USTRUCT(BlueprintType, DisplayName = "컨트롤 로테이션 기준 모션 워핑")
struct MNYS_API FYSMotionWarpParam_ControlRotation : public FYSMotionWarpParam
{
	GENERATED_BODY()
	
	virtual bool GetWarpTargetData(const UYSGameplayAbility* InAbility, FMotionWarpingTarget& OutTarget) const override;
};

USTRUCT(BlueprintType, DisplayName = "타겟 기준으로 모션 워핑 ( 회전 ) ")
struct MNYS_API FYSMotionWarpParam_Target : public FYSMotionWarpParam
{
	GENERATED_BODY()

	virtual bool GetWarpTargetData(const UYSGameplayAbility* InAbility, FMotionWarpingTarget& OutTarget) const override;
	
public : 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "타겟을 향할 것인가?")
	bool bFaceTarget = true;
};

// 타겟 앞까지 반드시 이동한다. 회전만 하는 모션은 '타겟 기준 (회전)' 파라미터를 쓴다.
USTRUCT(BlueprintType, DisplayName = "타겟 앞까지 모션 워핑 ( 위치 · 회전 )")
struct MNYS_API FYSMotionWarpParam_TargetFront : public FYSMotionWarpParam
{
	GENERATED_BODY()

	virtual bool GetWarpTargetData(const UYSGameplayAbility* InAbility, FMotionWarpingTarget& OutTarget) const override;
	virtual bool RequiresTranslation() const override { return true; }

public :
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "타겟 앞 정지 거리 (cm)", ClampMin = "0"))
	float StopDistance = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "이동 상한 (cm)", ClampMin = "0"))
	float MaxDistance = 800.f;
};
