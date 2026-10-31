// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Ability/YSGameplayAbility.h"
#include "YSGameplayAbility_AIBase.generated.h"

class UEnvQuery;
class UYSAbilitySystemComponent;
class UYSAIAbilityScoreFunctionBase;
class UYSGameplayAbility_AIBase;
struct FYSTargetingActorCollections;

struct FYSAIAbilityScore
{
	const UYSGameplayAbility_AIBase* Ability = nullptr;
	FGameplayAbilitySpecHandle Handle;
	TArray<TPair<const UYSAIAbilityScoreFunctionBase*, float>> Factors;
	float Factor = 1.f;
	float Score = 0.f;
	bool bIsActive = false;
	bool bCanActivate = false;

	bool IsCandidate() const;
};

struct FYSAIAbilitySelection
{
	TArray<FYSAIAbilityScore> Scores;
	int32 BestIndex = INDEX_NONE;

	const FYSAIAbilityScore* GetBest() const { return Scores.IsValidIndex(BestIndex) ? &Scores[BestIndex] : nullptr; }
};

UCLASS()
class MNYS_API UYSGameplayAbility_AIBase : public UYSGameplayAbility
{
	GENERATED_BODY()

public :
	// 점수 = 가중치 × 고려 항목 곱. 평가기 · 디버거는 이 결과를 부르기만 한다.
	static FYSAIAbilitySelection SelectAbility(UYSAbilitySystemComponent* ASC);

	FYSAIAbilityScore EvaluateScore(const FGameplayAbilitySpec& Spec, const FGameplayAbilityActorInfo* ActorInfo, const FYSTargetingActorCollections* TargetingCollections) const;

	virtual void SetupPlayBack(const FGameplayEventData* TriggerEventData) override;

	UEnvQuery* GetQueryToReposition() const;
public :
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, Category = "YS | Condition", meta = (DisplayName = "AI용 Utility Score"))
	TArray<TObjectPtr<UYSAIAbilityScoreFunctionBase>> UtilityScore;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | Condition", meta = (DisplayName = "AI용 Utility Score Weight"))
	float BaseUtilityScore = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | Condition", meta = (DisplayName = "AI용 Utility Score Threshold"))
	float UtilityScoreThreshold = 50.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | EQS", meta = (DisplayName = "EQS Query"))
	TSoftObjectPtr<UEnvQuery> QueryToReposition;
};
