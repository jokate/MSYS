// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "YSAIAbilityScoreFunction.generated.h"

struct FYSTargetingActorCollections;
struct FGameplayAbilityActorInfo;
/**
 * 
 */
UCLASS(Abstract, EditInlineNew, DefaultToInstanced, CollapseCategories)
class MNYS_API UYSAIAbilityScoreFunctionBase : public UObject
{
	GENERATED_BODY()

	
public : 
	virtual float GetScoreFactor(const FGameplayAbilityActorInfo* ActorInfo, const FYSTargetingActorCollections* TargetingActorCollections) const
	{
		return 1.f;
	}
};

UCLASS(DisplayName = "거리 대조")
class MNYS_API UYSAIAbilityScoreFunction_Distance : public UYSAIAbilityScoreFunctionBase
{
	GENERATED_BODY()

public:
	virtual float GetScoreFactor(const FGameplayAbilityActorInfo* ActorInfo, const FYSTargetingActorCollections* TargetingActorCollections) const override;

public : 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | AI", meta = (DisplayName = " 거리"))
	float Distance;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | AI", meta = (DisplayName = "범위 내"))
	bool bIsRange;
}; 
