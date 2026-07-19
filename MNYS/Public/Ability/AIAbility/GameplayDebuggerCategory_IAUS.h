// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#if WITH_GAMEPLAY_DEBUGGER_MENU
#include "Runtime/GameplayDebugger/Public/GameplayDebuggerCategory.h"
class MNYS_API FGameplayDebuggerCategory_IAUS : public FGameplayDebuggerCategory
{
public:
	// Sets default values for this actor's properties
	FGameplayDebuggerCategory_IAUS();

	virtual void CollectData(APlayerController* OwnerPC, AActor* DebugActor) override;
	virtual void DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext) override;

	static TSharedRef<FGameplayDebuggerCategory> MakeInstance();
	
protected :     
	struct FRepData
	{
		struct FScoreFactorDebug
		{
			FString FunctionName;
			float Factor = 0.f;
		};

		struct FAbilityScoreDebug
		{
			FString AbilityName;
			float UtilityScore = 0.f;    // BaseUtilityScore × 팩터 곱 (가중치 포함)
			float ScoreFactor = 0.f;    // 팩터 곱 (가중치 제외)
			float Threshold = 0.f;
			bool bCanActivate = false;
			bool bIsActive = false;
			TArray<FScoreFactorDebug> Factors;
		};

		TArray<FAbilityScoreDebug> Abilities;
		int32 BestAbilityIndex = INDEX_NONE;

		void Serialize(FArchive& Ar);
	};
	FRepData DataPack;
};

#endif