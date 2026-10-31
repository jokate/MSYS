// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify/YSAnimNotify_SendGameplayEvent.h"

void UYSAnimNotify_SendGameplayEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                             const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	TriggerGameplayData.SendToActor(MeshComp->GetOwner());
}
