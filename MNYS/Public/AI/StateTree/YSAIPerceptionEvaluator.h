// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"
#include "YSAIPerceptionEvaluator.generated.h"

/**
 * 
 */

class UEnvQuery;

UENUM(BlueprintType)
enum class EYSTargetingPolicy : uint8
{
	Fixed,
	Dynamic,
};

USTRUCT()
struct FYSAITargetInstancedData
{
	GENERATED_BODY()

public :
	void Reset()
	{
		TargetActor = nullptr;
	}
	
	// 타겟팅 되는 액터 산정.
	UPROPERTY(VisibleAnywhere, Category = "Output")
	TObjectPtr<AActor> TargetActor;
	
	UPROPERTY(EditAnywhere, Category = "Input | Parameters")
	TObjectPtr<UEnvQuery> TargetingEQS;
};

USTRUCT(DisplayName = "타겟팅 될 액터 찾기.")
struct MNYS_API FYSAIFindTargetEvaluator :  public FStateTreeEvaluatorCommonBase
{
	
	GENERATED_BODY()
	
public : 
	FYSAIFindTargetEvaluator();
	using FInstanceDataType = FYSAITargetInstancedData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual void TreeStart(FStateTreeExecutionContext& Context) const override;
	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

protected : 
	// Invalid 한 경우 ( 실패 한 경우에 타겟을 놓을 것인가? 예를 들어서 퍼셉션 된 타겟이 없다면의 가정 조치. )
	UPROPERTY(EditAnywhere, Category = "Input | Parameters")
	bool NeedToReleaseInvalidResult = false;
	
	UPROPERTY(EditAnywhere, Category = "Input | Parameters")
	EYSTargetingPolicy Policy = EYSTargetingPolicy::Dynamic;
	
	UPROPERTY(EditAnywhere, Category = "Input | Parameters", meta = (EditCondition = "Policy == EYSTargetingPolicy::Dynamic", EditConditionHides))
	float SearchInterval = 0.5f;
	
private : 
	void _SearchBestTarget(const FStateTreeExecutionContext& Context) const;
	
	mutable float CurrentSearchTime = 0.f;
};	