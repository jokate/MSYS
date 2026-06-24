// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AttackableActor/YSTraceObject.h"
#include "General/YSStruct.h"
#include "YSAT_Trace.generated.h"

/**
 * UYSTraceObject 를 GAS AbilityTask 로 감싸는 얇은 래퍼입니다.
 * 트레이스 핵심 로직은 UYSTraceObject 에 있으며,
 * 이 클래스는 Task 수명 관리와 OnTraceHit 델리게이트 포워딩만 담당합니다.
 */
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
	void _OnTraceObjectHit(const TArray<FHitResult>& HitResults, const FName& DamageRow);

	UFUNCTION()
	void _OnHitCountDepleted();
private:
	UPROPERTY()
	TObjectPtr<UYSTraceObject> TraceObject;

	FYSTraceConfig TraceConfig;
};
