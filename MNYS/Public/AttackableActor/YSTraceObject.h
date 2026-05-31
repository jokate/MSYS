// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "General/YSStruct.h"
#include "UObject/Object.h"
#include "YSTraceObject.generated.h"

/**
 * 순수 트레이스 로직 컨테이너.
 * UAbilityTask / AActor / UObject 등 어떤 컨텍스트에서도 사용할 수 있도록
 * GAS·Ability 의존성을 완전히 제거한 UObject 레이어입니다.
 *
 * 소유자가 직접 Tick(DeltaTime)을 호출하거나,
 * bTraceOnce 용도라면 ExecuteOnce()를 한 번 호출하면 됩니다.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTraceHit, const TArray<FHitResult>&, HitResults, const FName&, DamageRow);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHitCountDepleted);

UCLASS()
class MNYS_API UYSTraceObject : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * 팩토리 메서드.
	 * @param InOuter      GC 수명 관리용 Outer (소유 Actor 혹은 AbilityTask 권장)
	 * @param InOwnerActor 트레이스 기준이 되는 Actor (소켓·위치 조회에 사용)
	 * @param InConfig     트레이스 설정
	 */
	static UYSTraceObject* Create(UObject* InOuter, AActor* InOwnerActor, const FYSTraceConfig& InConfig);

	/** 매 프레임 소유자 Tick에서 호출 */
	void ExecuteOnce();
	void Tick(float DeltaTime);

	/** 히트 상태 초기화 (TracedActors / HitTimeMap 리셋) */
	void ResetHitState();

	/** 소유 Actor 교체 — Projectile이 반사 등으로 소유자가 바뀔 경우 대비 */
	void SetOwnerActor(AActor* InActor) { OwnerActor = InActor; }

	UPROPERTY(BlueprintAssignable)
	FOnTraceHit OnTraceHit;
	
	UPROPERTY(BlueprintAssignable)
	FOnHitCountDepleted OnHitCountDepleted;
	
protected : 
	bool IsHitCountDepleted() const;

private:
	void _TraceByConfig();

private:
	UPROPERTY()
	TObjectPtr<AActor> OwnerActor;

	FYSTraceConfig TraceConfig;

	UPROPERTY()
	TArray<TObjectPtr<AActor>> TracedActors;

	UPROPERTY()
	TMap<TObjectPtr<AActor>, float> HitTimeMap;

	UPROPERTY()
	int32 CurrentHitProcessCount;
	
	FVector PreviousLocation = FVector::ZeroVector;
	bool bHasPreviousLocation = false;
};
