// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "General/YSEnum.h"
#include "StructUtils/InstancedStruct.h"
#include "YSContextSwitchVariant.generated.h"

class UAnimMontage;

// 새 타겟이 플레이어보다 어디 있는가 (02_COMBAT_S4_IMPL_SPEC §3 높이 축).
UENUM(BlueprintType)
enum class EYSContextHeightBand : uint8
{
	Below	UMETA(DisplayName = "아래"),
	Level	UMETA(DisplayName = "동일"),
	Above	UMETA(DisplayName = "공중"),
};

// 전환 직전 캐릭터 정면과 새 타겟 방향의 각 (§3 방향 축).
UENUM(BlueprintType)
enum class EYSContextDirection : uint8
{
	Front	UMETA(DisplayName = "정면"),
	Side	UMETA(DisplayName = "측면"),
	Rear	UMETA(DisplayName = "후면"),
};

// 변형의 이동 (§4 이동 열).
UENUM(BlueprintType)
enum class EYSContextSwitchMove : uint8
{
	Turn	UMETA(DisplayName = "회전 (제자리에서 타겟 쪽으로)"),
	Warp	UMETA(DisplayName = "워프 (타겟 앞까지)"),
	Dash	UMETA(DisplayName = "돌진 (모션 루트모션 그대로)"),
};

/**
 * 변형 시작 때 한 번 잰 상황. 축마다 구간 하나다.
 */
struct MNYS_API FYSContextSwitchSituation
{
	FGameplayTagContainer TargetTags;
	EYSContextHeightBand Height = EYSContextHeightBand::Level;
	EYSTargetDistanceBand Distance = EYSTargetDistanceBand::OutOfRange;
	EYSContextDirection Direction = EYSContextDirection::Front;
};

/**
 * 상황 판정 경계. 사양표 §3 의 임시 후보값이 기본값이다.
 * 거리 경계는 여기 없다 — 캐릭터 전투 프로필(UYSCombatProfile)의 거리 구간을 그대로 쓴다.
 */
USTRUCT()
struct MNYS_API FYSContextSwitchJudgeRules
{
	GENERATED_BODY()

	// 새 타겟 Z − 플레이어 Z 가 +이 값을 넘으면 공중, −이 값보다 낮으면 아래.
	UPROPERTY(EditAnywhere, Category = "YS | ContextSwitch", meta = (DisplayName = "높이 경계 (cm)", ClampMin = "0"))
	float HeightThreshold = 100.f;

	UPROPERTY(EditAnywhere, Category = "YS | ContextSwitch", meta = (DisplayName = "정면 각 상한 (도)", ClampMin = "0", ClampMax = "180"))
	float FrontAngle = 60.f;

	// 이 각을 넘으면 후면.
	UPROPERTY(EditAnywhere, Category = "YS | ContextSwitch", meta = (DisplayName = "측면 각 상한 (도)", ClampMin = "0", ClampMax = "180"))
	float SideAngle = 120.f;

	EYSContextHeightBand ResolveHeight(float DeltaZ) const;
	EYSContextDirection ResolveDirection(const FVector& Forward, const FVector& ToTarget) const;
};

/**
 * 컨텍스트 스위치 공격의 변형 하나 (02_COMBAT_S4_IMPL_SPEC §4).
 * 진입기 형(FYSSwitchEntryForm)과 섞지 않는다. 새 상황은 표에 행을 더한다.
 * 고르는 쪽은 UYSGameplayAbility_ContextSwitch, 치는 쪽은 UYSPlaybackTask_ContextSwitchHit.
 */
USTRUCT(BlueprintType)
struct MNYS_API FYSContextSwitchVariant : public FTableRowBase
{
	GENERATED_BODY()

public :
	bool AcceptsInput(const FGameplayTag& InputTag) const { return InputTag.IsValid() && InputTag.MatchesAny(TriggerInputs); }

	/** 비운 축은 무관으로 보고, 채운 축은 모두 맞아야 한다. */
	bool Matches(const FYSContextSwitchSituation& Situation) const;

	/** 상태 > 높이 > 거리 > 방향. 채운 축이 높은 순위일수록 크다. 여러 행이 맞으면 큰 쪽을 고른다. */
	int32 GetSpecificity() const;

	/**
	 * 표에서 이 입력 · 상황에 맞는 행. 없으면 NAME_None (변형 없이 평소 입력대로).
	 * InputMatcher 는 행의 입력 태그를 받아 이번 입력인지 답한다.
	 */
	static FName FindBestRow(const UDataTable* Table, TFunctionRef<bool(const FYSContextSwitchVariant&)> InputMatcher, const FYSContextSwitchSituation& Situation);

public :
	// 스위치 창 안의 이 입력에만 나간다 (상태 접두 없는 원본: Input.Attack · Input.Skill1 · Input.Skill2).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Condition", meta = (DisplayName = "입력", Categories = "Input"))
	FGameplayTagContainer TriggerInputs;

	// 상태 축. 둘 다 비우면 무관. '일반'은 제외 태그에 브레이크 등을 넣어 나타낸다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Condition", meta = (DisplayName = "타겟 상태 — 필요 태그"))
	FGameplayTagContainer TargetRequiredTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Condition", meta = (DisplayName = "타겟 상태 — 제외 태그"))
	FGameplayTagContainer TargetExcludedTags;

	// 아무것도 고르지 않으면 무관.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Condition", meta = (DisplayName = "높이", Bitmask, BitmaskEnum = "/Script/MNYS.EYSContextHeightBand"))
	int32 HeightMask = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Condition", meta = (DisplayName = "거리", Bitmask, BitmaskEnum = "/Script/MNYS.EYSTargetDistanceBand"))
	int32 DistanceMask = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Condition", meta = (DisplayName = "방향", Bitmask, BitmaskEnum = "/Script/MNYS.EYSContextDirection"))
	int32 DirectionMask = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Motion", meta = (DisplayName = "몽타주"))
	TSoftObjectPtr<UAnimMontage> Montage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Motion", meta = (DisplayName = "이동"))
	EYSContextSwitchMove Move = EYSContextSwitchMove::Turn;

	// 몽타주의 모션 워프 노티파이와 같은 이름. 회전 · 워프에 쓴다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Motion", meta = (DisplayName = "워프 타깃 이름", EditCondition = "Move != EYSContextSwitchMove::Dash", EditConditionHides))
	FName WarpTargetName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Motion", meta = (DisplayName = "타겟 앞 정지 거리 (cm)", ClampMin = "0", EditCondition = "Move == EYSContextSwitchMove::Warp", EditConditionHides))
	float WarpStopDistance = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Motion", meta = (DisplayName = "워프 이동 상한 (cm)", ClampMin = "0", EditCondition = "Move == EYSContextSwitchMove::Warp", EditConditionHides))
	float WarpMaxDistance = 800.f;

	UPROPERTY(EditAnywhere, Category = "YS | Hit", meta = (DisplayName = "판정 도형", BaseStruct = "/Script/MNYS.YSTargetingShape", ExcludeBaseStruct))
	FInstancedStruct Shape;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Hit", meta = (DisplayName = "대상 규칙"))
	EYSSwitchEntryTargetRule TargetRule = EYSSwitchEntryTargetRule::CurrentTarget;

	// 변형 시작부터 판정까지.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Hit", meta = (DisplayName = "판정 지연 (초)", ClampMin = "0"))
	float HitDelay = 0.2f;

	// 피해 · 히트 반응 · 그로기 피해(등급 2)는 피해 행이 갖는다. 브레이크 배율도 피해 행의 상태별 배율이다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Hit", meta = (DisplayName = "피해 행", GetOptions = "MNYS.YSDeveloperSettings.GetDamageRowOptions"))
	FName DamageRow = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Hit", meta = (DisplayName = "임팩트"))
	EYSImpactGrade Impact = EYSImpactGrade::M;

	// 등급 0 · 1 은 그로기 게이지가 없어 그로기 피해 대신 경직이다. 비우면 피해 행을 그대로 쓴다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Stagger", meta = (DisplayName = "등급 0·1 피해 행 (경직 종류)", GetOptions = "MNYS.YSDeveloperSettings.GetDamageRowOptions"))
	FName StaggerDamageRow = NAME_None;

	// 등급 0 · 1 에게 거는 경직 GE 길이. 남용 감쇠 배율이 곱해진다. 0 이면 걸지 않는다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Stagger", meta = (DisplayName = "등급 0·1 경직 길이 (초)", ClampMin = "0"))
	float StaggerDuration = 0.4f;
};
