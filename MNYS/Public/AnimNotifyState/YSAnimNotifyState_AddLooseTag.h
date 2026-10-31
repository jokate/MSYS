// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "YSAnimNotifyState_AddLooseTag.generated.h"

UCLASS(DisplayName = "태그 추가")
class MNYS_API UYSAnimNotifyState_AddLooseTag : public UAnimNotifyState
{
	GENERATED_BODY()

public :
	virtual void PostLoad() override;
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

public :
	UPROPERTY(EditAnywhere, Category = "Tag | Custom | Target To Add Tag")
	FGameplayTagContainer CustomTags;

private :
	// 옛 저장 값을 읽기만 한다. 불러올 때 CustomTags 로 옮긴다.
	UPROPERTY()
	bool bBlockInput = false;

	UPROPERTY()
	bool bAcceptAbilityInput = false;
};
