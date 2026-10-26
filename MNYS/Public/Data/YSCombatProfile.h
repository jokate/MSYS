// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "General/YSEnum.h"
#include "General/YSStruct.h"
#include "YSCombatProfile.generated.h"

/**
 * 캐릭터별 타게팅 수치. 규칙(후보 · 점수 · 순환 · 구간)은 UYSCombatTargetComponent 가 갖고,
 * 캐릭터마다 다른 값은 여기에만 둔다 — 캐릭터가 늘면 이 애셋이 하나 늘 뿐이다.
 * 모든 기본값은 임시값이다.
 */
UCLASS(BlueprintType)
class MNYS_API UYSCombatProfile : public UPrimaryDataAsset
{
	GENERATED_BODY()

public :
	UYSCombatProfile();

	/** 각도(0~180)가 MaxAngle 이하인 첫 구간. 없으면 nullptr. */
	const FYSTurnBand* FindTurnBand(float AbsDeltaYaw) const;

	/** 거리 구간을 고른다. Previous 와 다른 구간으로 넘어갈 때는 경계 이력 여유만큼 더 가야 넘어간다. */
	EYSTargetDistanceBand ResolveDistanceBand(float Distance, EYSTargetDistanceBand Previous) const;

	/** 이력 없이 경계값만으로 고른 구간. */
	EYSTargetDistanceBand RawBand(float Distance) const;

public :
#pragma region Candidate
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | Candidate", meta = (DisplayName = "공격 사정거리 (cm)", ClampMin = "0"))
	float AttackRange = 1500.f;

	// 카메라 정면 기준 전체 각도.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | Candidate", meta = (DisplayName = "시야각 (도)", ClampMin = "0", ClampMax = "360"))
	float ViewAngle = 120.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | Candidate", meta = (DisplayName = "제외 상태 태그"))
	FGameplayTagContainer ExcludedTags;
#pragma endregion

#pragma region Priority
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | Priority", meta = (DisplayName = "가중치: 화면 중심각", ClampMin = "0"))
	float AngleWeight = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | Priority", meta = (DisplayName = "가중치: 거리", ClampMin = "0"))
	float DistanceWeight = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | Priority", meta = (DisplayName = "가중치: 직전 타겟 유지", ClampMin = "0"))
	float KeepWeight = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | Priority", meta = (DisplayName = "가중치: 위협", ClampMin = "0"))
	float ThreatWeight = 0.5f;

	// 대상이 이 태그 중 하나라도 가지면 위협 점수를 받는다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | Priority", meta = (DisplayName = "위협 판정 태그"))
	FGameplayTagContainer ThreatTags;
#pragma endregion

#pragma region Switch
	// 조건을 벗어난 타겟을 이 시간 안에 다시 조건 안으로 들어오면 그대로 유지한다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | Switch", meta = (DisplayName = "이탈 후 유지 시간 (초)", ClampMin = "0"))
	float LoseGraceTime = 1.f;
#pragma endregion

#pragma region Awareness
	// 주변 적 감지. 반경은 공격 사정거리를 그대로 쓴다. 360 이면 각도 검사를 끈다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | Awareness", meta = (DisplayName = "감지 시야각 (도)", ClampMin = "0", ClampMax = "360"))
	float AwarenessViewAngle = 360.f;

	// 감지된 적이 0명인 채로 이 시간이 지나면 교전이 끝난다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | Awareness", meta = (DisplayName = "교전 종료 대기 (초)", ClampMin = "0"))
	float EngagementEndDelay = 3.f;
#pragma endregion

#pragma region DistanceBand
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | DistanceBand", meta = (DisplayName = "근 구간 상한 (cm)", ClampMin = "0"))
	float NearMax = 300.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | DistanceBand", meta = (DisplayName = "중 구간 상한 (cm)", ClampMin = "0"))
	float MidMax = 800.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | DistanceBand", meta = (DisplayName = "원 구간 상한 (cm)", ClampMin = "0"))
	float FarMax = 1500.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | DistanceBand", meta = (DisplayName = "경계 이력 여유 (cm)", ClampMin = "0"))
	float BandHysteresis = 50.f;
#pragma endregion

#pragma region Turn
	// 작은 각부터 MaxAngle 오름차순으로 둔다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | Turn", meta = (DisplayName = "회전 각도 구간"))
	TArray<FYSTurnBand> TurnBands;

	// 행이 없는 출처는 즉시 회전한다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | Turn", meta = (DisplayName = "출처별 회전 규칙"))
	TMap<EYSFaceYawSource, FYSTurnSourceRule> TurnSourceRules;

	// 이 태그 중 하나라도 있는 동안은 '막는 태그 따름' 출처의 회전이 대기한다. 카메라는 먼저 돈다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | Turn", meta = (DisplayName = "회전을 막는 상태 태그"))
	FGameplayTagContainer TurnBlockedTags;

	// 입력 상태 머신에 이 상태 요청이 하나라도 있으면 막는다. 태그와 둘 중 하나만 걸려도 막는다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | Turn", meta = (DisplayName = "회전을 막는 입력 상태"))
	TArray<EYSInputStatesType> TurnBlockedInputStates = { EYSInputStatesType::Attack, EYSInputStatesType::Dodge };

	// 끄면 이동 중 요청은 턴 몽타주 없이 보간만 한다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | Turn", meta = (DisplayName = "이동 중에도 턴 몽타주 사용"))
	bool bTurnMontageWhileMoving = false;

	// 턴 몽타주의 모션 워핑 창이 쓰는 워프 타깃 이름.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | Turn", meta = (DisplayName = "턴 워프 타깃 이름"))
	FName TurnWarpTargetName = TEXT("FaceYaw");
#pragma endregion
};
