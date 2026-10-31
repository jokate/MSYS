// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "General/YSEnum.h"
#include "YSGroggyRules.generated.h"

class UAnimMontage;

/** 그로기 게이지 → 브레이크 규칙의 수치. 규칙은 UYSGroggyComponent 가 갖고 적마다 다른 값은 여기에만 둔다. 모든 기본값은 임시값이다. */
USTRUCT(BlueprintType)
struct MNYS_API FYSGroggyRules
{
	GENERATED_BODY()

	FYSGroggyRules();

#pragma region Damage
	// 피해 행의 그로기 피해(FYSDamageInfo::GroggyDamage)에 곱한다. 등급 · 적마다 다른 값은 이 하나로 맞춘다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Damage", meta = (DisplayName = "받는 그로기 피해 배율", ClampMin = "0"))
	float GroggyDamageTakenMultiplier = 1.f;
#pragma endregion

#pragma region Break
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Break", meta = (DisplayName = "브레이크 시간 (초)", ClampMin = "0"))
	float BreakDuration = 4.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Break", meta = (DisplayName = "브레이크 중 받는 피해 배율", ClampMin = "0"))
	float DamageTakenMultiplier = 1.5f;

	// State.Groggy.Broken 은 늘 붙는다. 여기는 그 밖에 더 붙일 태그 (기본: AI.Suppress 로 무방비).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Break", meta = (DisplayName = "브레이크 중 더 부여할 태그"))
	TArray<FGameplayTag> GrantedTags;

	// 브레이크 순간 진행 중인 어빌리티를 취소한다. 이 태그를 가진 어빌리티는 남긴다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Break", meta = (DisplayName = "취소하지 않을 어빌리티 태그"))
	TArray<FGameplayTag> KeepAbilitiesWithTags;

	// 비면 재생하지 않는다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Break", meta = (DisplayName = "브레이크 몽타주"))
	TSoftObjectPtr<UAnimMontage> BreakMontage;

#pragma endregion

#pragma region Presentation
	// 진입 Cue 가 켜져 있는 시간. 브레이크 시간은 이 정지가 끝난 뒤부터 센다. 진입 Cue 의 시간 왜곡 트랙을 길이 0(큐가 꺼질 때까지)으로 두면 이 값 하나로 맞는다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Presentation", meta = (DisplayName = "진입 정지 (초)", ClampMin = "0"))
	float EntryHoldTime = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Presentation", meta = (DisplayName = "회복 예고 (끝나기 전 초)", ClampMin = "0"))
	float WarningLeadTime = 1.f;

	// 단계마다 적에게 켜고 끄는 Cue (Instigator = 그로기를 깬 쪽). 무엇을 보이고 들려줄지는 Cue 애셋이 정한다. 빈 단계는 건너뛴다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Presentation", meta = (DisplayName = "단계별 Cue", Categories = "GameplayCue"))
	TMap<EYSBreakPhase, FGameplayTag> PhaseCues;
#pragma endregion

#pragma region Recover
	// 최대치 = 초기 최대치 × (1 + min(브레이크 횟수 × 증가율, 상한)). 페이즈 전환 때 초기화.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Recover", meta = (DisplayName = "브레이크당 최대치 증가율", ClampMin = "0"))
	float MaxGrowthPerBreak = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Recover", meta = (DisplayName = "최대치 증가 상한", ClampMin = "0"))
	float MaxGrowthCap = 0.6f;

	// 브레이크가 끝난 뒤 이 시간 동안 그로기 피해를 받지 않는다 (오버드라이브 연속 브레이크 방지).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Recover", meta = (DisplayName = "브레이크 후 유예 (초)", ClampMin = "0"))
	float PostBreakGrace = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Recover", meta = (DisplayName = "회복 시작 지연 (초)", ClampMin = "0"))
	float RegenDelay = 3.f;

	// 평타의 균형 DPS 보다 낮아야 평타로도 깨진다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Recover", meta = (DisplayName = "초당 회복 (최대치 비율)", ClampMin = "0"))
	float RegenRatePerSecond = 0.08f;
#pragma endregion
};

/** 보스 · 엘리트별 그로기 규칙. 몬스터에 지정하지 않으면 YS Developer Settings 의 기본 규칙을 쓴다. */
UCLASS(BlueprintType)
class MNYS_API UYSGroggyProfile : public UPrimaryDataAsset
{
	GENERATED_BODY()

public :
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | Groggy", meta = (ShowOnlyInnerProperties))
	FYSGroggyRules Rules;
};
