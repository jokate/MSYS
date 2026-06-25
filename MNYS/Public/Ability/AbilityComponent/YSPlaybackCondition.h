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
};

// 태그 체크 조건
USTRUCT(DisplayName = "컨텍스트 태그 조건")
struct FYSPlaybackCondition_ContextTag : public FYSPlaybackCondition
{
	GENERATED_BODY()
    
	UPROPERTY(EditAnywhere, Category = "YS | Condition", meta = (DisplayName = "체크할 태그"))
	FGameplayTag RequiredTag;

	virtual bool Evaluate(const TSharedPtr<FYSPlaybackContext>& Context) const override
	{
		return Context->ContextTags.HasTagExact(RequiredTag);
	}
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