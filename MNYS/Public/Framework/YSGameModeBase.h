// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
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
	
	bool operator==(const UObject* Object) const
	{
		return DilationCaller == Object;
	}
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
	
	void RegisterTimeDilation(UObject* Object, float TimeDilation);
	void RemoveTimeDilation(UObject* Object);
	virtual void Tick(float DeltaSeconds) override;
private : 
	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> TimeDilationActors;
	
	UPROPERTY()
	TArray<FYSTimeDilationFactor> TimeDilationFactors;
};
