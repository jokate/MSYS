// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Ability/YSGameplayAbility.h"
#include "YSGameplayAbility_ContextSwitch.generated.h"

struct FYSContextSwitchVariant;

/**
 * 컨텍스트 스위치 공격 (02_COMBAT_S4_IMPL_SPEC). Q/E 수동 전환이 연 스위치 창 안 입력(Switch.Input.*)에 부여한다.
 *
 * 발동 직전에 새 타겟 상황으로 변형 표의 행을 고른다. 창을 연 게 Q/E 가 아니거나, 맞는 행이 없거나, 행의 몽타주가 없으면
 * 발동하지 않고, 스위치 창 규칙의 '못 받으면 아래 상태로 넘길 입력'으로 창 아래 상태가 평소 공격을 낸다.
 * 몽타주는 FYSMontageSelector_ContextSwitch, 이동 · 판정은 UYSPlaybackTask_ContextSwitchHit 이 이 행을 읽는다.
 *
 * 플레이백 그래프 · 레거시 배열이 비면 이 기본을 쓴다: 노드 1개 = 몽타주 재생(변형 몽타주) + 변형 판정,
 * 몽타주 완료 · 중단 → 체인 종료. 그래프를 넣으면 그쪽이 우선한다.
 */
UCLASS()
class MNYS_API UYSGameplayAbility_ContextSwitch : public UYSGameplayAbility
{
	GENERATED_BODY()

public :
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	const FYSContextSwitchVariant* GetSelectedVariant() const;
	FName GetSelectedRow() const { return SelectedRow; }
	AActor* GetVariantTarget() const { return VariantTarget.Get(); }

	/** 변형 시작 때 한 번 읽은 남용 감쇠 배율. 피해 · 그로기 피해 · 경직 길이에 곱한다. */
	float GetEffectScale() const { return EffectScale; }

protected :
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void BuildDefaultPlaybacks(TArray<TObjectPtr<UYSAbilityPlaybackBase>>& OutPlaybacks) override;

private :
	/** 이 스펙의 입력 · 수동 전환 기록 · 새 타겟 상황으로 행을 고른다. 못 고르면 NAME_None. */
	FName ResolveRow(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, AActor*& OutTarget) const;

private :
	FName SelectedRow = NAME_None;
	TWeakObjectPtr<AActor> VariantTarget;
	float EffectScale = 1.f;
};
