// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "YSAT_Trace.generated.h"

/**
 * 
 */

class AYSTA_Trace;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTraceResultDelegate, const FGameplayAbilityTargetDataHandle&,
                                            TargetDataHandle);
UCLASS()
class MNYS_API UYSAT_Trace : public UAbilityTask
{
	GENERATED_BODY()
	
public :
	UYSAT_Trace(const FObjectInitializer& ObjectInitializer);
	
	static UYSAT_Trace* CreateTask(UGameplayAbility* OwningAbility, const FName& TargetDamageInfo, const AActor* DamageCauser );

	virtual void Activate() override;

	virtual void OnDestroy(bool bInOwnerFinished) override;

	void SetupTraceData();

	AYSTA_Trace* SpawnAndInitializeTargetActor(TSubclassOf<AYSTA_Trace> TraceClass );

	void FinalizeTargetActor(AYSTA_Trace* TraceTarget, const FTransform& FinalizeTransform, const FName& AttachmentSocket = NAME_None);
protected:
	
	void OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& DataHandle);


public :

	UPROPERTY(BlueprintAssignable)
	FTraceResultDelegate OnComplete;
	
	UPROPERTY()
	TArray<AYSTA_Trace*> SpawnedTargetActors; 

	UPROPERTY()
	FName TargetDamageInfo;

	UPROPERTY()
	const AActor* TargetActor;
};
