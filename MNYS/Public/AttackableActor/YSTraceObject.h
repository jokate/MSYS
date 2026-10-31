// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "General/YSStruct.h"
#include "UObject/Object.h"
#include "YSTraceObject.generated.h"

struct FYSAbilityHitContext;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTraceEnded);

/** 트레이스 수명(시작 · 매 프레임 · 끝)을 맡는다. 어빌리티 태스크와 공격 액터가 같이 쓴다. 맞히면 히트 기록으로 낸 어빌리티에 알린다. */
UCLASS()
class MNYS_API UYSTraceObject : public UObject
{
	GENERATED_BODY()

public:
	/** InOuter 는 GC 수명용(소유 액터 · 태스크), InOwnerActor 는 소켓 · 위치 기준. 소유자가 OnTraceEnded 를 묶은 뒤 Start 를 부른다. */
	static UYSTraceObject* Create(UObject* InOuter, AActor* InOwnerActor, AActor* InInstigator, const FYSTraceConfig& InConfig,
		float InEffectScale, const TSharedPtr<FYSAbilityHitContext>& InHitContext);

	/** bTraceOnce 면 한 번 판정하고 바로 끝난다. */
	void Start();
	void Tick(float DeltaTime);
	void Stop();

	/** 히트 상태 초기화 (TracedActors / HitTimeMap 리셋) */
	void ResetHitState();

	/** 소유 Actor 교체 — Projectile이 반사 등으로 소유자가 바뀔 경우 대비 */
	void SetOwnerActor(AActor* InActor) { OwnerActor = InActor; }
	
	bool DecreaseHitProcessCount();

	/** 한 번 판정을 마쳤거나 히트 수를 다 썼을 때. */
	UPROPERTY(BlueprintAssignable)
	FOnTraceEnded OnTraceEnded;
	
protected : 
	bool IsHitCountDepleted() const;
	void _End();

private:
	void _TraceByConfig();
	void _ProcessValidHit(const TArray<FHitResult>& InProcessedHits);
private:
	UPROPERTY()
	TObjectPtr<AActor> OwnerActor;

	UPROPERTY()
	TObjectPtr<AActor> Instigator;
	
 	FYSTraceConfig TraceConfig;

	// 피해 · 그로기 피해에 곱한다. 어빌리티가 아니면 1.
	float EffectScale = 1.f;

	TWeakPtr<FYSAbilityHitContext> HitContext;

	bool bEnded = false;

	UPROPERTY()
	TArray<TObjectPtr<AActor>> TracedActors;

	UPROPERTY()
	TMap<TObjectPtr<AActor>, float> HitTimeMap;

	/** 저스트 회피 GE 를 이미 준 대상. 창이 닫히면 AdmitHit 이 지운다. */
	TSet<TWeakObjectPtr<AActor>> JustAvoidGrantedActors;

	UPROPERTY()
	int32 RemainHitProcessCount;

	FVector PreviousLocation = FVector::ZeroVector;
	FRotator EffectRotation = FRotator::ZeroRotator;
	bool bHasPreviousLocation = false;
};
