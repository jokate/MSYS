// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "General/YSStruct.h"
#include "YSAT_PlaySequence.generated.h"

class ALevelSequenceActor;
class ULevelSequencePlayer;
/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FYSSequencePlayFinishedDelegate);

UCLASS()
class MNYS_API UYSAT_PlaySequence : public UAbilityTask
{
	GENERATED_BODY()

public : 
	static UYSAT_PlaySequence* CreatePlaySequenceTask(UGameplayAbility* OwningAbility, const FYSSequencePlaySettings& InSettings, AActor* Instigator, AActor* Target);
	
	virtual void Activate() override;
	virtual void OnDestroy(bool bInOwnerFinished) override;
	
	UFUNCTION()
	void OnSequenceFinished();
	
	UPROPERTY(BlueprintAssignable)
	FYSSequencePlayFinishedDelegate OnSequenceFinishedDelegate;

protected :
	
	UPROPERTY()
	TObjectPtr<ULevelSequencePlayer> LevelSequencePlayer;

	UPROPERTY()
	TObjectPtr<ALevelSequenceActor> LevelSequenceActor;

	UPROPERTY()
	FYSSequencePlaySettings SequenceSettings;

	UPROPERTY()
	TObjectPtr<AActor> InstigatorActor;
	
	UPROPERTY()
	TObjectPtr<AActor> TargetActor;
};
