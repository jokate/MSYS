// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "StateTreeEvaluatorBase.h"
#include "AI/StateTree/Condition/YSAIStateTreeConditionInstanceData.h"
#include "UObject/Object.h"
#include "YSStateTreeEvaluator.generated.h"

class UYSAbilitySystemComponent;
class UAbilitySystemComponent;
class AYSMonster;
class UEnvQuery;
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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Output")
	TObjectPtr<AActor> BestTarget = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Output")
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


USTRUCT(BlueprintType)
struct FYSStateTree_AbilityActivation
{
	GENERATED_BODY()
	
public : 
	// 사용 가능한 최고의 어빌리티..
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Output")
	FGameplayAbilitySpecHandle BestAbilitySpecHandle; 
	
	// 어빌리티 사용 가능 여부
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Output")
	bool bIsEnableActivateAbility = false;
	
	// 만약 어빌리티 사용을 하려고 했는데 실패한 경우, Reposition을 잡아주기 위한 포지셔닝 EQS
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Output")
	TObjectPtr<UEnvQuery> EnvQueryInstance = nullptr;
};

USTRUCT(DisplayName = "스킬 사용 관련 정보 캐싱")
struct MNYS_API FYSStateTreeEvaluatorAbilityActivation : public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()
	
public : 
	using FInstanceDataType = FYSStateTree_AbilityActivation;
	
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual void TreeStart(FStateTreeExecutionContext& Context) const override;
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual void Tick(FStateTreeExecutionContext& Context, float DeltaTime) const override;
	
protected:
	TStateTreeExternalDataHandle<AYSAIController> AIControllerHandle;
	
	
	mutable TWeakObjectPtr<UYSAbilitySystemComponent> YSASC;
	mutable TWeakObjectPtr<AActor> OwnerPawn;
};