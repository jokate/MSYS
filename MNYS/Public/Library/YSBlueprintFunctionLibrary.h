// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "General/YSEnum.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "YSBlueprintFunctionLibrary.generated.h"

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
	
	UFUNCTION(BlueprintCallable)
	static FRotator GetAbilityEventRotation(EYSDirectionPolicy DirectionPolicy, UYSGameplayAbility* OwningAbility, const FName& SocketName); 
	
	UFUNCTION(BlueprintCallable)
	static FVector GetAbilityEventPosition(EYSPositionPolicy PositionPolicy, UYSGameplayAbility* OwningAbility, 
		const FName& SocketName, const FVector& RelativeOffset);
};

