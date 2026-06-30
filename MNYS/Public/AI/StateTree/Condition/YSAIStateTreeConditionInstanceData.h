// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "UObject/Object.h"
#include "YSAIStateTreeConditionInstanceData.generated.h"

/**
 * 
 */

class AYSAIController;

USTRUCT()
struct FYSTargetingActorInstancedData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	bool bInvert = false;
};

USTRUCT()
struct MNYS_API FYSAIStateTreeConditionBase : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()
	
public : 
	virtual AYSAIController* GetAIControllerFromContext(FStateTreeExecutionContext& Context) const;
};


USTRUCT(DisplayName = "타겟팅 액터가 존재하는가?")
struct MNYS_API FYSTargetingActorCondition : public FYSAIStateTreeConditionBase
{
	GENERATED_BODY()
	
public : 
	using UInstanceDataType = FYSTargetingActorInstancedData;
	
	virtual const UStruct* GetInstanceDataType() const override { return UInstanceDataType::StaticStruct(); }
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
	
};