// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotifyState/YSAnimNotifyState_SendGameplayTagEvent.h"
#include "General/YSStruct.h"
#include "YSAnimNotify_SendGameplayEvent.generated.h"

/**
 * 
 */
UCLASS()
class MNYS_API UYSAnimNotify_SendGameplayEvent : public UAnimNotify
{
	GENERATED_BODY()

public :
	void SendGameplayEvent(USkeletalMeshComponent* MeshComp, const FGameplayEventSendData& InTriggerGameplayData);
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
public :
	UPROPERTY(EditAnywhere, meta = (DisplayName = "시작 시 전송할 게임 플레이 이벤트"))
	FGameplayEventSendData TriggerGameplayData;
};
