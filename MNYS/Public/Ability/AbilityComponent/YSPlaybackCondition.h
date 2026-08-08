// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "General/YSStruct.h"
#include "UObject/Object.h"
#include "YSPlaybackCondition.generated.h"

/**
 * 
 */
// 조건 베이스
USTRUCT()
struct FYSPlaybackCondition
{
	GENERATED_BODY()
	virtual ~FYSPlaybackCondition() = default;
	
	virtual bool Evaluate(const TSharedPtr<FYSPlaybackContext>& Context) const { return true; }
	virtual void OnConditionEvaluatedComplete(const TSharedPtr<FYSPlaybackContext>& Context) const {}
};

// 태그 체크 조건
USTRUCT(DisplayName = "컨텍스트 태그 조건")
struct FYSPlaybackCondition_ContextTag : public FYSPlaybackCondition
{
	GENERATED_BODY()
    
	UPROPERTY(EditAnywhere, Category = "YS | Condition", meta = (DisplayName = "체크할 태그"))
	FGameplayTagContainer RequiredTags;
	
	UPROPERTY(EditAnywhere, Category = "YS | Condition", meta = (DisplayName = "반전"))
	bool bInvert = false;
	
	UPROPERTY(EditAnywhere, Category = "YS | Condition", meta = (DisplayName = ""))
	EYSOperatorType OperatorType = EYSOperatorType::OR;
	
	virtual bool Evaluate(const TSharedPtr<FYSPlaybackContext>& Context) const override;
};

USTRUCT(DisplayName = "버프 존재 여부")
struct FYSPlaybackCondition_HasGameplayEffect : public FYSPlaybackCondition
{
	GENERATED_BODY()
	
public : 
	UPROPERTY(EditAnywhere, Category = "YS | Condition", meta = (DisplayName = "체크할 GE 클래스"))
	TSubclassOf<class UGameplayEffect> GameplayEffect;
	
	virtual bool Evaluate(const TSharedPtr<FYSPlaybackContext>& Context) const override;
};

USTRUCT(DisplayName = "타겟 액터가 각도 내에 있는지 여부")
struct FYSPlaybackCondition_ActorInArc : public FYSPlaybackCondition
{
	GENERATED_BODY()

public : 
	virtual bool Evaluate(const TSharedPtr<FYSPlaybackContext>& Context) const override;
	
public : 
	UPROPERTY(EditAnywhere, Category = "YS | Condition", meta = (DisplayName = "체크할 각도"))
	float ArcAngle = 90.f;
};

USTRUCT(DisplayName = "타겟 액터에 대한 거리 체크")
struct FYSPlaybackCondition_ActorInDistance : public FYSPlaybackCondition
{
	GENERATED_BODY()
	
public : 
	virtual bool Evaluate(const TSharedPtr<FYSPlaybackContext>& Context) const override;
	
public : 
	UPROPERTY(EditAnywhere, Category = "YS | Condition", meta = (DisplayName = "체크할 거리"))
	float CheckDistance = 1000.f;
	
	UPROPERTY(EditAnywhere, Category = "YS | Condition", meta = (DisplayName = "거리 내?"))
	bool bIsInRange = false;
};

USTRUCT(DisplayName = "인풋 관련 조건")
struct FYSPlaybackCondition_Input : public FYSPlaybackCondition
{
	GENERATED_BODY()
	
public : 
	virtual bool Evaluate(const TSharedPtr<FYSPlaybackContext>& Context) const override;
	
	// 통과된 케이스에서만 처리하자.
	virtual void OnConditionEvaluatedComplete(const TSharedPtr<FYSPlaybackContext>& Context) const override;
public : 
	UPROPERTY(EditAnywhere, Category = "YS | Condition", meta = (DisplayName = "체크할 인풋"))
	FYSInputHistory InputHistory;

	UPROPERTY(EditAnywhere, Category = "YS | Condition", meta = (DisplayName = "반전"))
	bool bInvert = false;
};

/**
 * 버튼이 지금 눌려 있는지 묻는다.
 *
 * FYSPlaybackCondition_Input과 달리 소비하지 않는다 — 홀드는 사건이 아니라 상태이기 때문이다.
 * 연사 루프(발사 완료 → 여전히 누르고 있으면 자기 자신으로)에 쓴다.
 */
USTRUCT(DisplayName = "인풋 홀드 중인지 여부")
struct FYSPlaybackCondition_InputHeld : public FYSPlaybackCondition
{
	GENERATED_BODY()

public :
	virtual bool Evaluate(const TSharedPtr<FYSPlaybackContext>& Context) const override;
	// OnConditionEvaluatedComplete를 구현하지 않는다 — 상태 질의라 소비할 것이 없다.

public :
	// 상태 접두(Idle., Aim.)가 붙지 않은 원본 태그를 넣는다. 예: Input.Aim
	UPROPERTY(EditAnywhere, Category = "YS | Condition", meta = (DisplayName = "홀드 확인할 인풋 (상태 접두 없는 원본)", Categories = "Input"))
	FGameplayTag InputTag;

	UPROPERTY(EditAnywhere, Category = "YS | Condition", meta = (DisplayName = "반전"))
	bool bInvert = false;
};