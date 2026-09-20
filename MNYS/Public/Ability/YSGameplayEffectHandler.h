// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "YSGameplayEffectHandler.generated.h"

struct FGameplayEffectModifierMagnitude;
class UGameplayEffect;
/**
 * 
 */
USTRUCT(BlueprintType)
struct MNYS_API FYSGameplayEffectHandler
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | GameplayEffect", meta = (DisplayName = "게임 플레이 이펙트"))
	TSubclassOf<UGameplayEffect> EffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | GameplayEffect", meta = (DisplayName = "Set By Caller 값"))
	TMap<FGameplayTag, float> Magnitudes;

#if WITH_EDITOR
	void CollectSetByCallerTags(const UGameplayEffect* Effect, TArray<FGameplayTag>& OutTags) const;
	bool SyncFromEffect();
	bool IsInSync() const;
	static bool SyncAllIn(const UStruct* Type, void* Container);
	void AddIfSetByCaller(const FGameplayEffectModifierMagnitude& ModifierMagnitude, TArray<FGameplayTag>& OutTag) const;
#endif
};
