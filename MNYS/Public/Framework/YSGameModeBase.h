// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "General/YSEnum.h"
#include "YSGameModeBase.generated.h"

/**
 *
 */
USTRUCT(BlueprintType)
struct FYSTimeDilationFactor
{
	GENERATED_BODY()

public :
	UPROPERTY()
	UObject* DilationCaller = nullptr;

	UPROPERTY()
	float TimeDilation = 0.f;

	UPROPERTY()
	EYSTimeDilationScope Scope = EYSTimeDilationScope::All;

	// Scope 를 판단하는 기준 액터 (큐 대상 · 때린 쪽).
	UPROPERTY()
	TWeakObjectPtr<AActor> ScopeOwner;

	// Participants 에서 기준 액터와 함께 걸리는 액터 (맞은 쪽).
	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> ScopeActors;

	bool operator==(const UObject* Object) const
	{
		return DilationCaller == Object;
	}

	bool AppliesTo(const AActor* Actor) const;
};

UCLASS()
class MNYS_API AYSGameModeBase : public AGameMode
{
	GENERATED_BODY()

public :

	void RegisterTimeDilationActor(AActor* Actor)
	{
		TimeDilationActors.Add(Actor);
	}

	void UnregisterTimeDilationActor(AActor* Actor)
	{
		TimeDilationActors.Remove(Actor);
	}

	void SetTimeDilation();

	void RegisterTimeDilation(UObject* Object, float TimeDilation, EYSTimeDilationScope Scope = EYSTimeDilationScope::All, AActor* ScopeOwner = nullptr, const TArray<AActor*>& ScopeActors = TArray<AActor*>());
	void RemoveTimeDilation(UObject* Object);

	// 길이가 정해진 시간 왜곡. 월드 타이머로 풀리므로 왜곡 중에도 제때 끝난다. 임팩트의 히트스톱 · 슬로우가 이 길로 걸린다.
	void StartTimedDilation(float TimeDilation, float Duration, EYSTimeDilationScope Scope, AActor* ScopeOwner, const TArray<AActor*>& ScopeActors);
	virtual void Tick(float DeltaSeconds) override;
private :
	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> TimeDilationActors;

	UPROPERTY()
	TArray<FYSTimeDilationFactor> TimeDilationFactors;
};
