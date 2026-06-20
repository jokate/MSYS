// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "General/YSEnum.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "YSBlueprintFunctionLibrary.generated.h"

struct FYSSpawnActorConfig;
class UYSGameplayAbility;
class UAbilitySystemComponent;
class UYSCharacterAttributeSetBase;

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

	static FRotator GetEventRotation(EYSDirectionPolicy DirectionPolicy, AActor* OwnerActor, const FName& SocketName, const FRotator& RelativeOffset, AActor* PlaybackTarget = nullptr);
	static FVector GetEventPosition(EYSPositionPolicy PositionPolicy, AActor* OwnerActor, const FName& SocketName, const FVector& RelativeOffset);

	UFUNCTION(BlueprintCallable)
	static FRotator GetAbilityEventRotation(EYSDirectionPolicy DirectionPolicy, UYSGameplayAbility* OwningAbility, const FName& SocketName, const FRotator& RelativeOffset);

	UFUNCTION(BlueprintCallable)
	static FVector GetAbilityEventPosition(EYSPositionPolicy PositionPolicy, UYSGameplayAbility* OwningAbility,
		const FName& SocketName, const FVector& RelativeOffset);
	
	static AActor* SpawnByConfig(UObject* WorldContext, const FYSSpawnActorConfig& Config, AActor* OwnerActor, AActor* TargetActor, AActor* AttachParent = nullptr);
	static FTransform CalculateSpawnTransform(UObject* WorldContext, const FYSSpawnActorConfig& Config, AActor* OwnerActor, AActor* TargetActor);

};

