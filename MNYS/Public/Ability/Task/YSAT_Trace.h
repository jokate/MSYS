// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "General/YSStruct.h"
#include "YSAT_Trace.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTraceHit, const TArray<FHitResult>&, HitResults, const FName&, DamageRow);

UCLASS()
class MNYS_API UYSAT_Trace : public UAbilityTask
{
	GENERATED_BODY()

public :
	UYSAT_Trace();
	static UYSAT_Trace* CreateTask(UGameplayAbility* InAbility, const FYSTraceConfig& InConfig );
	virtual void TickTask(float DeltaTime) override;
	virtual void Activate() override;

private :
	void _TraceByConfig();;

public :
	UPROPERTY(BlueprintAssignable)
	FOnTraceHit OnTraceHit;
	

private :
	UPROPERTY()
	TArray<AActor*> TracedActors;

	// bContinuousHit 모드: 액터별 마지막 히트 시각 (WorldTimeSeconds)
	UPROPERTY()
	TMap<AActor*, float> HitTimeMap;

	// Sweep 시작점 — Activate() 이후 첫 Tick에서 초기화됨
	FVector PreviousSocketLocation = FVector::ZeroVector;
	bool bHasPreviousLocation = false;

	FYSTraceConfig TraceConfig;
};
