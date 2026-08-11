// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "YSSpawnInitializable.generated.h"

struct FYSAbilityHitContext;
// This class does not need to be modified.
UINTERFACE()
class UYSSpawnInitializable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MNYS_API IYSSpawnInitializable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void OnSpawnInitialize(AActor* InOwnerActor, const TSharedPtr<FYSAbilityHitContext>& HitContext) = 0;
};
