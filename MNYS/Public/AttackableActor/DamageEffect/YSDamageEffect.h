// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "General/YSStruct.h"
#include "YSDamageEffect.generated.h"


class UGameplayEffect;

USTRUCT()
struct FYSDamageEffectContext
{
	GENERATED_BODY()
	
	UPROPERTY()
	TObjectPtr<AActor> Source = nullptr;

	/** 피해의 주체. 스탯·팀 판정의 주인. */
	UPROPERTY()
	TObjectPtr<AActor> Instigator = nullptr;

	UPROPERTY()
	TObjectPtr<AActor> Target = nullptr;

	UPROPERTY()
	FHitResult HitResult;
};

USTRUCT(BlueprintType)
struct MNYS_API FYSDamageEffectBase
{
	GENERATED_BODY()

	virtual ~FYSDamageEffectBase() = default;
	virtual void Apply(const FYSDamageEffectContext& Context) const {}

	bool CanApply(const FYSDamageEffectContext& Context) const;

	// 비면 늘 적용한다. 예: 스위치 어택 적중 강조는 State.Groggy.Broken.
	UPROPERTY(EditAnywhere, Category = "YS | Damage Effect", meta = (DisplayName = "대상이 가져야 할 태그 (모두)"))
	FGameplayTagContainer RequiredTargetTags;
};

/** 대상에게 Cue 를 한 번 실행한다 (Execute). 트랙형 YS Cue 액터는 Execute 를 받지 않으니 Burst 계열 Cue 애셋을 쓴다. */
USTRUCT(DisplayName = "게임 플레이 큐 실행")
struct FYSDamageEffect_GameplayCue : public FYSDamageEffectBase
{
	GENERATED_BODY()

	virtual void Apply(const FYSDamageEffectContext& Context) const override;

	UPROPERTY(EditAnywhere, Category = "YS | Damage Effect", meta = (DisplayName = "큐", Categories = "GameplayCue"))
	FGameplayTag CueTag;
};

USTRUCT(DisplayName = "넉백")
struct FYSDamageEffect_Knockback : public FYSDamageEffectBase
{
	GENERATED_BODY()

	virtual void Apply(const FYSDamageEffectContext& Context) const override;

	UPROPERTY(EditAnywhere, Category = "YS | Damage Effect", meta = (DisplayName = "밀어내는 세기"))
	float Strength = 900.f;

	UPROPERTY(EditAnywhere, Category = "YS | Damage Effect", meta = (DisplayName = "띄우는 세기"))
	float Lift = 0.f;

	UPROPERTY(EditAnywhere, Category = "YS | Damage Effect", meta = (DisplayName = "히트 지점 기준 (끄면 소스 위치 기준)"))
	bool bUseImpactNormal = false;
};


USTRUCT(DisplayName = "게임 플레이 이펙트 적용")
struct FYSDamageEffect_ApplyGameplayEffect : public FYSDamageEffectBase
{
	GENERATED_BODY()

	virtual void Apply(const FYSDamageEffectContext& Context) const override;

	UPROPERTY(EditAnywhere, Category = "YS | Damage Effect", meta = (DisplayName = "히트 타겟에게 적용할 게임 플레이 이펙트"))
	TSubclassOf<UGameplayEffect> GameplayEffectToTarget;
	
	UPROPERTY(EditAnywhere, Category = "YS | Damage Effect", meta = (DisplayName = "히트 주체에게 적용할 게임 플레이 이펙트"))
	TSubclassOf<UGameplayEffect> GameplayEffectToInstigator;

	UPROPERTY(EditAnywhere, Category = "YS | Damage Effect", meta = (DisplayName = "GE 레벨"))
	float Level = 1.f;
};

USTRUCT(DisplayName = "타격 지점 액터 스폰")
struct FYSDamageEffect_SpawnActor : public FYSDamageEffectBase
{
	GENERATED_BODY()

	virtual void Apply(const FYSDamageEffectContext& Context) const override;

	UPROPERTY(EditAnywhere, Category = "YS | Damage Effect", meta = (DisplayName = "스폰 설정"))
	FYSSpawnActorConfig Config;

	UPROPERTY(EditAnywhere, Category = "YS | Damage Effect", meta = (DisplayName = "타겟에 어태치"))
	bool bAttachToTarget = false;
};