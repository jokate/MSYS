// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "General/YSEnum.h"
#include "StructUtils/InstancedStruct.h"
#include "YSSwitchEntryForm.generated.h"

/**
 * 스위치 어택 진입기의 형 하나 (A · B · 기본). 세 형은 캐릭터 진입기 모션 1개를 같이 쓰고 이 행만 다르다.
 * 형이 늘면 표에 행이 하나 는다. 게이지 · 태그 쿨 · 전역 쿨 · 무적/창 길이는 스쿼드 · 남용 정책 · 창 GE 쪽이다.
 * 고르는 쪽은 UYSPlaybackTask_SwitchEntry.
 */
USTRUCT(BlueprintType)
struct MNYS_API FYSSwitchEntryForm : public FTableRowBase
{
	GENERATED_BODY()

public :
	bool AcceptsInput(const FGameplayTag& InputTag) const { return TriggerInputs.IsEmpty() == false && InputTag.MatchesAny(TriggerInputs); }
	bool IsNoInputForm() const { return TriggerInputs.IsEmpty(); }

	/** 타겟 · 거리 조건을 채우는가. 못 채우면 대체 형으로 간다. */
	bool CanUse(const AActor* Instigator, const AActor* Target) const;

	/** 판정 도형과 대상 규칙으로 맞힐 대상을 고른다. 아군 · 사망 · 무적은 뺀다. */
	void CollectTargets(AActor* Instigator, AActor* Target, TArray<AActor*>& OutTargets) const;

	/** 대상마다 피해 행 히트 · 그로기 피해(남용 정책 배율 곱)를 준다. */
	void ApplyToTargets(AActor* Instigator, const TArray<AActor*>& Targets) const;

	void SetWarp(AActor* Instigator, const AActor* Target) const;
	void ReleaseWarp(AActor* Instigator) const;

public :
	// 스위치 창 안에서 이 입력이 들어오면 이 형. 비워 두면 창이 닫힐 때까지 무입력일 때의 형이다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Input", meta = (DisplayName = "발동 입력"))
	FGameplayTagContainer TriggerInputs;

	UPROPERTY(EditAnywhere, Category = "YS | Hit", meta = (DisplayName = "판정 도형", BaseStruct = "/Script/MNYS.YSTargetingShape", ExcludeBaseStruct))
	FInstancedStruct Shape;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Hit", meta = (DisplayName = "대상 규칙"))
	EYSSwitchEntryTargetRule TargetRule = EYSSwitchEntryTargetRule::AllInShape;

	// 0 이면 거리 조건 없음. 현재 타겟 1체 규칙은 타겟이 없으면 조건 실패다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Hit", meta = (DisplayName = "타겟 최대 거리 (cm)", ClampMin = "0"))
	float MaxTargetDistance = 0.f;

	// 타겟 · 거리 조건을 못 채우면 이 행으로 나간다. 게이지는 이미 썼다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Hit", meta = (DisplayName = "조건 실패 시 대체 형"))
	FName FallbackForm = NAME_None;

	// 입력(무입력 형은 창 닫힘)부터 판정까지. 입력이 늦을수록 판정이 무적 밖으로 밀린다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Hit", meta = (DisplayName = "판정 지연 (초)", ClampMin = "0"))
	float HitDelay = 0.f;

	// 피해 · 히트 반응 · 넉백 · 경직 GE 는 피해 행이 갖는다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Hit", meta = (DisplayName = "피해 행", GetOptions = "MNYS.YSDeveloperSettings.GetDamageRowOptions"))
	FName DamageRow = NAME_None;

	// 적 하나당. 남용 정책 ② 배율이 곱해진다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Hit", meta = (DisplayName = "그로기 피해", ClampMin = "0"))
	float GroggyDamage = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Move", meta = (DisplayName = "이동"))
	EYSSwitchEntryMove Move = EYSSwitchEntryMove::RootMotion;

	// 캐릭터 진입기 몽타주의 모션 워프 노티파이와 같은 이름이어야 한다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Move", meta = (DisplayName = "워프 타깃 이름", EditCondition = "Move == EYSSwitchEntryMove::WarpToTarget", EditConditionHides))
	FName WarpTargetName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Move", meta = (DisplayName = "타겟 앞 정지 거리 (cm)", ClampMin = "0", EditCondition = "Move == EYSSwitchEntryMove::WarpToTarget", EditConditionHides))
	float WarpStopDistance = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Move", meta = (DisplayName = "워프 이동 상한 (cm)", ClampMin = "0", EditCondition = "Move == EYSSwitchEntryMove::WarpToTarget", EditConditionHides))
	float WarpMaxDistance = 800.f;
};
