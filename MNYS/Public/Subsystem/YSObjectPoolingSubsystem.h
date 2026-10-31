// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "YSObjectPoolingSubsystem.generated.h"

UCLASS()
class MNYS_API UYSObjectPoolingSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()
	
	
public :
	static UYSObjectPoolingSubsystem* Get(UWorld* World);
	
	virtual void Deinitialize() override;
	
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override { return true; }
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UYSObjectPoolingSubsystem, STATGROUP_Tickables); }

	AActor* GetPooledActor(TSubclassOf<AActor> ActorClass);

	/** 공격 액터 · 분신을 끝내는 길은 이것 하나다. 풀에서 나온 액터는 숨겨 돌려두고, 풀 밖 액터는 지운다. */
	static void ReturnToPool(AActor* Actor);

	void AddPooledActor(AActor* Actor);
	
	void RemoveInActivePoolActors();

protected : 
	bool IsPooled(const AActor* Actor) const;
	void ReturnPooledActor(AActor* Actor);

private : 
	// TMap 값으로 TArray 는 UHT 가 거부한다. 약참조라 UPROPERTY 로 얻을 GC 보호도 없다.
	TMap<TSubclassOf<AActor>, TArray<TWeakObjectPtr<AActor>>> PooledActors;
	
	// 2분마다 미사용 액터가 존재하는 케이스에는 놓아주자.
	UPROPERTY()
	float RefreshTime = 30.f;
	
	float AccumulatedTime = 0.f;
};
