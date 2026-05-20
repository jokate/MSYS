// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "General/YSEnum.h"
#include "YSAnimNotify_TransitionState.generated.h"

/**
 * 
 */
UCLASS()
class MNYS_API UYSAnimNotify_TransitionState : public UAnimNotify
{
	GENERATED_BODY()
	
public : 
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
public : 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "바꾸고자하는 State"))
	EYSInputStatesType NextState;
};
