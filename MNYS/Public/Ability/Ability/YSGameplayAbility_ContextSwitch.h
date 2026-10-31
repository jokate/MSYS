// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Ability/YSGameplayAbility.h"
#include "YSGameplayAbility_ContextSwitch.generated.h"

/**
 * Q/E 수동 전환이 연 스위치 창 안 입력(Switch.Input.*)에 부여한다. 모션은 그래프 첫 노드의 분기가 새 타겟 상황으로 고르고,
 * 맞는 경로가 없으면 발동하지 않아 창 아래 상태가 평소 공격을 낸다. 이동 · 판정은 몽타주 노티파이 → 이벤트 액션이 맡는다.
 */
UCLASS()
class MNYS_API UYSGameplayAbility_ContextSwitch : public UYSGameplayAbility
{
	GENERATED_BODY()

public :
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	/** 남용 감쇠 배율. 발동 때 한 번 읽는다. */
	virtual float GetEffectScale() const override { return EffectScale; }

protected :
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void SetupPlayBack(const FGameplayEventData* TriggerEventData) override;

private :
	/** 지금 수동 전환 창 안이면 그 전환의 타겟. 아니면 nullptr. */
	AActor* FindSwitchTarget(const FGameplayAbilityActorInfo* ActorInfo) const;

private :
	TWeakObjectPtr<AActor> SwitchTarget;
	float EffectScale = 1.f;
};
