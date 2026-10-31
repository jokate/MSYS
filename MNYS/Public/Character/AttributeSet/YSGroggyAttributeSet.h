// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "General/YSMacros.h"
#include "YSGroggyAttributeSet.generated.h"

/** 적의 그로기 게이지. 스위치 어택 · 평타 GE 가 IncomingGroggyDamage 로 깎고, 0 에 닿는 순간 Event.Groggy.Broken 만 보낸다 (브레이크 연출은 받는 쪽의 일). */
UCLASS()
class MNYS_API UYSGroggyAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public :
	UYSGroggyAttributeSet();

	ATTRIBUTE_ACCESSORS(UYSGroggyAttributeSet, Groggy);
	ATTRIBUTE_ACCESSORS(UYSGroggyAttributeSet, MaxGroggy);
	ATTRIBUTE_ACCESSORS(UYSGroggyAttributeSet, IncomingGroggyDamage);

	/** 그로기 게이지를 최대치의 Ratio 만큼으로 되돌린다 (다시 깎여 Broken 이 나게 하는 통로). 회복 시점 · 양은 부르는 쪽이 정한다. @return 대상에 게이지가 있어 되돌렸으면 true */
	UFUNCTION(BlueprintCallable, Category = "YS | Groggy")
	static bool RestoreGroggy(AActor* Target, float Ratio = 1.f);

	/** Amount × 대상 프로필의 받는 그로기 피해 배율만큼 그로기 피해 GE 를 건다. @return 대상에 게이지가 있어 걸었으면 true */
	static bool ApplyGroggyDamage(AActor* Instigator, AActor* Target, float Amount);

	/** 지금부터 Seconds 동안 그로기 피해를 버린다 (브레이크 후 유예). */
	void LockDamage(float Seconds);

	/** 마지막으로 그로기 피해를 받은 월드 시각. 회복 지연을 세는 쪽이 읽는다. */
	double GetLastDamageTime() const { return LastDamageTime; }

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

protected :
	UPROPERTY(BlueprintReadOnly, Category = "YS | Groggy", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Groggy;

	UPROPERTY(BlueprintReadOnly, Category = "YS | Groggy", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxGroggy;

	// 메타 어트리뷰트. 실행 후 Groggy 에 반영하고 0 으로 돌린다.
	UPROPERTY(BlueprintReadOnly, Category = "YS | Groggy", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData IncomingGroggyDamage;

private :
	double GetWorldTime() const;

	double LastDamageTime = -UE_BIG_NUMBER;
	double DamageLockedUntil = -UE_BIG_NUMBER;
};
