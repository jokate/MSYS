// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AttackableActor/YSTraceObject.h"
#include "General/YSStruct.h"
#include "YSAT_Trace.generated.h"

/** UYSTraceObject 를 GAS AbilityTask 로 감싸는 얇은 래퍼. 트레이스가 끝나면 태스크도 끝낸다. */
UCLASS()
class MNYS_API UYSAT_Trace : public UAbilityTask
{
	GENERATED_BODY()

public:
	UYSAT_Trace();

	static UYSAT_Trace* CreateTask(UGameplayAbility* InAbility, const FYSTraceConfig& InConfig);

	virtual void TickTask(float DeltaTime) override;
	virtual void Activate() override;
	virtual void OnDestroy(bool bInOwnerFinished) override;
private:
	UFUNCTION()
	void _OnTraceEnded();
private:
	UPROPERTY()
	TObjectPtr<UYSTraceObject> TraceObject;

	FYSTraceConfig TraceConfig;
};
