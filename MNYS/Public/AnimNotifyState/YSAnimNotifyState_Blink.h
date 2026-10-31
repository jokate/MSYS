// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Components/SkinnedMeshComponent.h"
#include "YSAnimNotifyState_Blink.generated.h"

/** 블링크(순간이동) 구간 동안 메시를 숨긴다. 숨겨 렌더링이 끊겨도 애님 틱 · 본 갱신이 유지되게 VisibilityBasedAnimTickOption 을 AlwaysTickPoseAndRefreshBones 로 강제한다. */
UCLASS(meta = (DisplayName = "YS 블링크 (메시 숨김)"))
class MNYS_API UYSAnimNotifyState_Blink : public UAnimNotifyState
{
	GENERATED_BODY()

public :
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

private :
	// 노티파이 객체는 애님 에셋당 1개를 모든 캐릭터가 공유하므로, 복원할 값은 컴포넌트별로 보관해야 한다
	TMap<TWeakObjectPtr<USkeletalMeshComponent>, EVisibilityBasedAnimTickOption> SavedTickOptions;
};
