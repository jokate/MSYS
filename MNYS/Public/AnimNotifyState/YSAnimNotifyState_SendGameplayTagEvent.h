// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "General/YSStruct.h"
#include "YSAnimNotifyState_SendGameplayTagEvent.generated.h"

/**
 * 
 */

class UYSAbilityTriggerPayload;

UCLASS(meta = (DisplayName = "게임 플레이 이벤트 송신"))
class MNYS_API UYSAnimNotifyState_SendGameplayTagEvent : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	void SendGameplayEvent(USkeletalMeshComponent* MeshComp, const FGameplayEventSendData& NotifyData);

public :
	UPROPERTY(EditAnywhere, meta = (DisplayName = "시작 시 전송할 게임 플레이 이벤트"))
	FGameplayEventSendData TriggerGameplayData;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "종료 시, 전송할 게임 플레이 이벤트"))
	FGameplayEventSendData EndTriggerGameplayData;
};
