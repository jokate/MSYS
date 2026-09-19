// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "General/YSEnum.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Templates/SubclassOf.h"
#include "YSBlueprintFunctionLibrary.generated.h"

struct FYSDamageEffectContext;
struct FYSDamageInfo;
struct FYSAbilityHitContext;
struct FYSSpawnActorConfig;
struct FYSTransformPolicyContext;
class UYSGameplayAbility;
class UAbilitySystemComponent;
class UYSCharacterAttributeSetBase;
class UWorld;

/**
 *
 */
UCLASS()
class MNYS_API UYSBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public :
	UFUNCTION(BlueprintPure)
	static float GetFinalDamage(const UYSCharacterAttributeSetBase* Owner, const UYSCharacterAttributeSetBase* Target, const FName& SkillID);

	UFUNCTION(BlueprintCallable)
	static void SendHitEventToTarget(AActor* Instigator, AActor* Target, const FName& SkillID);
	
	static void SpawnEffects(UObject* WorldContextObject, const FName& DamageRow, const FVector& Location, const FRotator& Rotation);
	
	/** 풀에서 꺼내거나 새로 스폰한다. 어느 쪽이든 FinishSpawning 까지 끝난 액터가 나온다. */
	static AActor* AcquirePooledActor(UWorld* World, TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform);
	
	static AActor* SpawnByConfig(UObject* WorldContext, const FYSSpawnActorConfig& Config, const FYSTransformPolicyContext& PolicyContext, AActor* AttachParent = nullptr, const TSharedPtr<FYSAbilityHitContext>& HitContext = nullptr);
	static FTransform CalculateSpawnTransform(UObject* WorldContext, const FYSSpawnActorConfig& Config, const FYSTransformPolicyContext& PolicyContext);
	static void ApplyHitEffects(AActor* Source, AActor* Instigator, AActor* Target, const FName& SkillID, const FHitResult& HitResult);
};

