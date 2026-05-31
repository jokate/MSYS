// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "YSAttackableActor.h"
#include "YSProjectile.generated.h"

class UProjectileMovementComponent;
struct FYSTraceConfig;
/**
 * 
 */
UCLASS()
class MNYS_API AYSProjectile : public AYSAttackableActor
{
	GENERATED_BODY()

public : 
	AYSProjectile();

protected : 
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;
};
