// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "YSDamagableActor.h"
#include "YSProjectile.generated.h"

class UProjectileMovementComponent;
struct FYSTraceConfig;
/**
 * 
 */
UCLASS()
class MNYS_API AYSProjectile : public AYSDamagableActor
{
	GENERATED_BODY()

public : 
	AYSProjectile();

protected : 
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;
};
