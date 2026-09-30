// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "General/YSStruct.h"
#include "UObject/Object.h"
#include "YSPlaybackAction.generated.h"

/**
 * 
 */
UCLASS(Abstract, EditInlineNew, DefaultToInstanced, CollapseCategories)
class MNYS_API UYSPlaybackAction : public UObject
{
	GENERATED_BODY()

public :
	virtual void Execute(const TSharedPtr<FYSPlaybackContext>& Context) const {}
};

UCLASS(DisplayName = "게임플레이 이벤트 발행")
class MNYS_API UYSPlaybackAction_SendGameplayEvent : public UYSPlaybackAction
{
	GENERATED_BODY()

public :
	virtual void Execute(const TSharedPtr<FYSPlaybackContext>& Context) const override;

public :
	UPROPERTY(EditAnywhere, Category = "YS | Event", meta = (DisplayName = "발행할 이벤트"))
	FGameplayEventSendData EventData;
};

UCLASS(DisplayName = "타겟: 첫 히트 대상")
class MNYS_API UYSPlaybackAction_FirstHitTarget : public UYSPlaybackAction
{
	GENERATED_BODY()

public :
	virtual void Execute(const TSharedPtr<FYSPlaybackContext>& Context) const override;
};

UCLASS(DisplayName = "타겟: 저스트 회피 공격자")
class MNYS_API UYSPlaybackAction_JustAvoidTarget : public UYSPlaybackAction
{
	GENERATED_BODY()

public :
	virtual void Execute(const TSharedPtr<FYSPlaybackContext>& Context) const override;
};

UCLASS(DisplayName = "버프 해제")
class MNYS_API UYSPlaybackAction_ReleaseBuff : public UYSPlaybackAction
{
	GENERATED_BODY()

public :
	virtual void Execute(const TSharedPtr<FYSPlaybackContext>& Context) const override;

public :
	UPROPERTY(EditAnywhere, Category = "YS | ReleaseBuff")
	FGameplayTagContainer BuffTags;
};