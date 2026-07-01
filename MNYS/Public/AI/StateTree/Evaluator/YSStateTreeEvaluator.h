// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"
#include "AI/StateTree/Condition/YSAIStateTreeConditionInstanceData.h"
#include "UObject/Object.h"
#include "YSStateTreeEvaluator.generated.h"

struct FYSTargetingActorCollections;
class AYSAIController;
/**
 * 
 */
USTRUCT(BlueprintType)
struct FYSStateTree_TargetInstancedData
{
	GENERATED_BODY()
	
public : 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting")
	TObjectPtr<AActor> BestTarget = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting")
	TArray<TObjectPtr<AActor>> SkillTargets;
	
	TSharedPtr<FYSTargetingActorCollections> TargetingActorCollections;
};

USTRUCT(DisplayName = "타겟팅 정보 가져오기")
struct MNYS_API FYSStateTreeEvaluatorTarget : public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()
	
public : 
	using FInstanceDataType = FYSStateTree_TargetInstancedData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual void TreeStart(FStateTreeExecutionContext& Context) const override;
	virtual void Tick(FStateTreeExecutionContext& Context, float DeltaTime) const override;
	virtual void TreeStop(FStateTreeExecutionContext& Context) const override;

protected:
	TStateTreeExternalDataHandle<AYSAIController> AIControllerHandle;
};
