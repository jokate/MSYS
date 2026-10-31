// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "GameplayTagContainer.h"
#include "General/YSStruct.h"
#include "UObject/Object.h"
#include "YSPlaybackCondition.generated.h"

USTRUCT()
struct MNYS_API FYSPlaybackCondition
{
	GENERATED_BODY()
	virtual ~FYSPlaybackCondition() = default;
	
	virtual bool Evaluate(const TSharedPtr<FYSPlaybackContext>& Context) const { return true; }
	virtual void OnConditionEvaluatedComplete(const TSharedPtr<FYSPlaybackContext>& Context) const {}
};

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

/** 타겟까지의 거리 구간(근/중/원)으로 같은 입력 안의 모션을 고른다. 경계값 · 떨림 방지 여유는 UYSCombatProfile 에 있고, 타겟이 없으면 "구간 밖". */
USTRUCT(DisplayName = "타겟 거리 구간")
struct MNYS_API FYSPlaybackCondition_TargetDistanceBand : public FYSPlaybackCondition
{
	GENERATED_BODY()

public :
	virtual bool Evaluate(const TSharedPtr<FYSPlaybackContext>& Context) const override;

public :
	UPROPERTY(EditAnywhere, Category = "YS | Condition", meta = (DisplayName = "구간"))
	EYSTargetDistanceBand Band = EYSTargetDistanceBand::Near;
};

/** 타겟이 가진 태그. 타겟은 재생 타겟, 없으면 전투 타겟. */
USTRUCT(DisplayName = "타겟 태그")
struct MNYS_API FYSPlaybackCondition_TargetTag : public FYSPlaybackCondition
{
	GENERATED_BODY()

public :
	virtual bool Evaluate(const TSharedPtr<FYSPlaybackContext>& Context) const override;

public :
	// 전부 있어야 한다.
	UPROPERTY(EditAnywhere, Category = "YS | Condition", meta = (DisplayName = "필요 태그"))
	FGameplayTagContainer RequiredTags;

	// 하나라도 있으면 실패.
	UPROPERTY(EditAnywhere, Category = "YS | Condition", meta = (DisplayName = "제외 태그"))
	FGameplayTagContainer ExcludedTags;
};

/** 타겟 높이 구간. 타겟은 재생 타겟, 없으면 전투 타겟. 타겟이 없으면 실패. */
USTRUCT(DisplayName = "타겟 높이")
struct MNYS_API FYSPlaybackCondition_TargetHeight : public FYSPlaybackCondition
{
	GENERATED_BODY()

public :
	virtual bool Evaluate(const TSharedPtr<FYSPlaybackContext>& Context) const override;

public :
	// 비우면 무관.
	UPROPERTY(EditAnywhere, Category = "YS | Condition", meta = (DisplayName = "높이", Bitmask, BitmaskEnum = "/Script/MNYS.EYSTargetHeightBand"))
	int32 HeightMask = 0;
};

/** 기준 정면에서 본 타겟 방향 구간. 타겟은 재생 타겟, 없으면 전투 타겟. 타겟이 없으면 실패. */
USTRUCT(DisplayName = "타겟 방향")
struct MNYS_API FYSPlaybackCondition_TargetDirection : public FYSPlaybackCondition
{
	GENERATED_BODY()

public :
	virtual bool Evaluate(const TSharedPtr<FYSPlaybackContext>& Context) const override;

public :
	// 비우면 무관.
	UPROPERTY(EditAnywhere, Category = "YS | Condition", meta = (DisplayName = "방향", Bitmask, BitmaskEnum = "/Script/MNYS.EYSTargetDirectionBand"))
	int32 DirectionMask = 0;

	UPROPERTY(EditAnywhere, Category = "YS | Condition", meta = (DisplayName = "기준 정면"))
	EYSForwardReference Reference = EYSForwardReference::Current;
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

/** 버튼이 지금 눌려 있는지 묻는다. FYSPlaybackCondition_Input 과 달리 소비하지 않는다 — 홀드는 사건이 아니라 상태다 (연사 루프용). */
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

USTRUCT(DisplayName = "자원 보유 여부")
struct FYSPlaybackCondition_HasResource : public FYSPlaybackCondition
{
	GENERATED_BODY()
	
public : 
	virtual bool Evaluate(const TSharedPtr<FYSPlaybackContext>& Context) const override;
public : 
	
	UPROPERTY(EditAnywhere, Category = "YS | Condition", meta = (DisplayName = "확인할 자원 어트리뷰트"))
	FGameplayAttribute ResourceAttribute;

	UPROPERTY(EditAnywhere, Category = "YS | Condition", meta = (DisplayName = "필요 수량"))
	float RequiredAmount = 1.f;
};