// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "YSDamageEffect.generated.h"

/**
 * 
 */

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
