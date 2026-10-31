// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "YSAnimNotifyState_InputState.generated.h"

/** 구간 동안 입력 상태를 건다. 시작에 표 행의 GE 를 걸고 끝에 뗀다. 몽타주가 끊기면 재생한 어빌리티의 종료 정리로도 뗀다. */
UCLASS(DisplayName = "입력 상태")
class MNYS_API UYSAnimNotifyState_InputState : public UAnimNotifyState
{
	GENERATED_BODY()

public :
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

public :
	UPROPERTY(EditAnywhere, Category = "YS | Input", meta = (DisplayName = "입력 상태", GetOptions = "MNYS.YSDeveloperSettings.GetInputStateOptions"))
	FName InputState;
};
