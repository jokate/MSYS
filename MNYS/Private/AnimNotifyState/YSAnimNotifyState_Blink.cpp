// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState/YSAnimNotifyState_Blink.h"

void UYSAnimNotifyState_Blink::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	MeshComp->SetHiddenInGame(false);
	Super::NotifyEnd(MeshComp, Animation, EventReference);
}

void UYSAnimNotifyState_Blink::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                           float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	MeshComp->SetHiddenInGame(true);
}
