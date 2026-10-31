// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState/YSAnimNotifyState_SendGameplayTagEvent.h"
#include "General/YSDefine.h"

void UYSAnimNotifyState_SendGameplayTagEvent::NotifyBegin(USkeletalMeshComponent* MeshComp,
                                                          UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	UE_LOG(LogYSMeasure, Verbose, TEXT("[Notify] t=%.3f Begin Anim=%s Duration=%.3f"), MeshComp->GetWorld()->GetTimeSeconds(), *GetNameSafe(Animation), TotalDuration);
	TriggerGameplayData.SendToActor(MeshComp->GetOwner());
}

void UYSAnimNotifyState_SendGameplayTagEvent::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	UE_LOG(LogYSMeasure, Verbose, TEXT("[Notify] t=%.3f End Anim=%s"), MeshComp->GetWorld()->GetTimeSeconds(), *GetNameSafe(Animation));
	EndTriggerGameplayData.SendToActor(MeshComp->GetOwner());
}
