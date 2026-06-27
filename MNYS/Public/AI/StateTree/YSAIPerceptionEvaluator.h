// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"
#include "YSAIPerceptionEvaluator.generated.h"

/**
 * 
 */

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
	
	
};

USTRUCT(DisplayName = "퍼셉션 타겟팅 된 액터 찾기.")
struct MNYS_API FYSAIPerceptionEvaluator :  public FStateTreeEvaluatorCommonBase
{
	
	GENERATED_BODY()
	
	using FInstanceDataType = FYSAITargetInstancedData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};
