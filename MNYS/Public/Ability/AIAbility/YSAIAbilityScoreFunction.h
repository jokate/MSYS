// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Curves/CurveFloat.h"
#include "UObject/Object.h"
#include "YSAIAbilityScoreFunction.generated.h"

struct FYSTargetingActorCollections;
struct FGameplayAbilityActorInfo;
/**
 * 어빌리티 유틸리티 스코어의 팩터 하나를 담당한다.
 * 파생 클래스는 GetInputValue()로 "측정"만 하고, 점수 매핑은 InputRange 정규화 + ResponseCurve가 담당한다.
 * GetInputValue()가 false를 반환하면 팩터 0 = 거부권(veto)으로 어빌리티 전체가 탈락한다.
 */
UCLASS(Abstract, EditInlineNew, DefaultToInstanced, CollapseCategories)
class MNYS_API UYSAIAbilityScoreFunctionBase : public UObject
{
	GENERATED_BODY()

public :
	float GetScoreFactor(const FGameplayAbilityActorInfo* ActorInfo, const FYSTargetingActorCollections* TargetingActorCollections) const;

protected :
	// 측정값을 OutRawInput에 담아 반환. false 반환 = 측정 불가 = 거부권(팩터 0)
	virtual bool GetInputValue(const FGameplayAbilityActorInfo* ActorInfo, const FYSTargetingActorCollections* TargetingActorCollections, float& OutRawInput) const
	{
		OutRawInput = InputRange.Max;
		return true;
	}

public :
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | AI", meta = (DisplayName = "입력 범위 (이 구간을 0~1로 정규화)"))
	FFloatInterval InputRange = FFloatInterval(0.f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | AI", meta = (DisplayName = "응답 커브 (입력 0~1 → 점수 0~1, 비어있으면 선형)"))
	FRuntimeFloatCurve ResponseCurve;
};

UCLASS(DisplayName = "거리 대조")
class MNYS_API UYSAIAbilityScoreFunction_Distance : public UYSAIAbilityScoreFunctionBase
{
	GENERATED_BODY()

protected:
	virtual bool GetInputValue(const FGameplayAbilityActorInfo* ActorInfo, const FYSTargetingActorCollections* TargetingActorCollections, float& OutRawInput) const override;
};
