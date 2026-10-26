// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "YSSwitchAbusePolicy.generated.h"

class AActor;

/**
 * 스위치 창 개시 기록. 정책은 const 설정값이라 상태는 이쪽(스쿼드 서브시스템 소유)에 둔다.
 */
struct MNYS_API FYSSwitchAbuseState
{
	float LastOpenTime = -BIG_NUMBER;

	TWeakObjectPtr<const AActor> LastTarget;

	// 같은 대상에게 연달아 연 횟수. 대상이 바뀌면 1 로 돌아간다.
	int32 ConsecutiveSameTarget = 0;

	TMap<TWeakObjectPtr<const AActor>, float> LastOpenTimeByTarget;

	// 대상마다 지금 창 바로 전에 연 시각. 창을 열 때 기록하므로 공격 시점에는 이쪽이 '재적용' 기준이다.
	TMap<TWeakObjectPtr<const AActor>, float> PreviousOpenTimeByTarget;

	void Record(const AActor* Target, float Now);
	float GetLastOpenTime(const AActor* Target) const;
	float GetPreviousOpenTime(const AActor* Target) const;
	bool IsSameAsLastTarget(const AActor* Target) const;
};

/**
 * 스위치 창 남용 제어. 기획 미결(S4 제어 장치 ① / ② / ③)을 구조체 하나씩으로 두고
 * YS Developer Settings 에서 갈아끼운다. 새 규칙이 오면 파생 구조체 하나를 더한다.
 */
USTRUCT()
struct MNYS_API FYSSwitchAbusePolicy
{
	GENERATED_BODY()
	virtual ~FYSSwitchAbusePolicy() = default;

	/** 창을 열어도 되는가 */
	virtual bool CanOpen(const FYSSwitchAbuseState& State, const AActor* Target, float Now) const { return true; }

	/** 이번에 소모할 게이지 */
	virtual float GetGaugeCost(const FYSSwitchAbuseState& State, const AActor* Target, float BaseCost) const { return BaseCost; }

	/** 스위치 어택의 경직 · 그로기 감소에 곱할 배율. 적중 쪽에서 읽는다. */
	virtual float GetEffectScale(const FYSSwitchAbuseState& State, const AActor* Target, float Now) const { return 1.f; }

	/** 시간으로 거는 제어(막기 · 감쇠)가 풀릴 때까지 남은 시간. 시간을 쓰지 않는 정책은 0. HUD 가 읽는다. */
	virtual float GetCooldownRemaining(const FYSSwitchAbuseState& State, const AActor* Target, float Now) const { return 0.f; }
};

/**
 * 플레이어 상태에 따라 기본 남용 정책 대신 쓸 정책 (예: 오버드라이브 중 전역 쿨 2초).
 * 정책을 비우면 그 상태에서는 제어를 건너뛴다 (전면 면제).
 */
USTRUCT()
struct MNYS_API FYSSwitchAbusePolicyOverride
{
	GENERATED_BODY()

	// 이 태그 중 하나라도 플레이어에게 있으면 이 행을 쓴다.
	UPROPERTY(EditAnywhere, Category = "YS | Switch", meta = (DisplayName = "조건 태그"))
	FGameplayTagContainer RequiredTags;

	UPROPERTY(EditAnywhere, Category = "YS | Switch", meta = (DisplayName = "정책 (비우면 면제)", BaseStruct = "/Script/MNYS.YSSwitchAbusePolicy", ExcludeBaseStruct))
	FInstancedStruct Policy;
};

USTRUCT(DisplayName = "① 전역 쿨타임")
struct MNYS_API FYSSwitchAbusePolicy_GlobalCooldown : public FYSSwitchAbusePolicy
{
	GENERATED_BODY()

	virtual bool CanOpen(const FYSSwitchAbuseState& State, const AActor* Target, float Now) const override;
	virtual float GetCooldownRemaining(const FYSSwitchAbuseState& State, const AActor* Target, float Now) const override;

	UPROPERTY(EditAnywhere, Category = "YS | Switch", meta = (DisplayName = "쿨타임 (초)", ClampMin = "0"))
	float Cooldown = 4.f;
};

USTRUCT(DisplayName = "② 대상별 재적용 감쇠")
struct MNYS_API FYSSwitchAbusePolicy_PerTargetDecay : public FYSSwitchAbusePolicy
{
	GENERATED_BODY()

	virtual float GetEffectScale(const FYSSwitchAbuseState& State, const AActor* Target, float Now) const override;
	virtual float GetCooldownRemaining(const FYSSwitchAbuseState& State, const AActor* Target, float Now) const override;

	UPROPERTY(EditAnywhere, Category = "YS | Switch", meta = (DisplayName = "감쇠 유지 시간 (초)", ClampMin = "0"))
	float DecayWindow = 6.f;

	UPROPERTY(EditAnywhere, Category = "YS | Switch", meta = (DisplayName = "감쇠 배율", ClampMin = "0", ClampMax = "1"))
	float DecayScale = 0.5f;
};

USTRUCT(DisplayName = "③ 같은 대상 연속 시 소모 증가")
struct MNYS_API FYSSwitchAbusePolicy_EscalatingCost : public FYSSwitchAbusePolicy
{
	GENERATED_BODY()

	virtual float GetGaugeCost(const FYSSwitchAbuseState& State, const AActor* Target, float BaseCost) const override;

	UPROPERTY(EditAnywhere, Category = "YS | Switch", meta = (DisplayName = "연속 1회당 추가 소모", ClampMin = "0"))
	float CostStep = 1.f;
};
