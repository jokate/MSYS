// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState/YSAnimNotifyState_Blink.h"

void UYSAnimNotifyState_Blink::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                           float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (IsValid(MeshComp) == false)
	{
		return;
	}

	// 숨겨져 렌더링이 끊겨도 몽타주/노티파이/본 갱신이 계속 돌도록 강제한다.
	// (기본 AlwaysTickPose는 포즈만 틱하고 본 갱신을 멈춰서 소켓 위치를 쓰는 노티파이가 어긋난다)
	SavedTickOptions.Add(MeshComp, MeshComp->VisibilityBasedAnimTickOption);
	MeshComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	MeshComp->SetHiddenInGame(true);
}

void UYSAnimNotifyState_Blink::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	if (IsValid(MeshComp))
	{
		MeshComp->SetHiddenInGame(false);

		EVisibilityBasedAnimTickOption SavedOption;
		if (SavedTickOptions.RemoveAndCopyValue(MeshComp, SavedOption))
		{
			MeshComp->VisibilityBasedAnimTickOption = SavedOption;
		}
	}

	Super::NotifyEnd(MeshComp, Animation, EventReference);
}
