// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Actor.h"
#include "YSGameplayCueNotify_Sequence.generated.h"

class ALevelSequenceActor;

UCLASS()
class MNYS_API AYSGameplayCueNotify_Sequence : public AGameplayCueNotify_Actor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AYSGameplayCueNotify_Sequence();
	virtual bool OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	virtual bool OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | GameplayCue", meta = (DisplayName = "재생할 레벨 시퀀스 액터"))
	TWeakObjectPtr<ALevelSequenceActor> SequenceToPlay;
private:
	UFUNCTION()
	void OnSequenceFinished();
	void StopSequenceSafely();
};
