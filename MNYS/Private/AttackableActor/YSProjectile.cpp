// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackableActor/YSProjectile.h"

#include "GameFramework/ProjectileMovementComponent.h"

AYSProjectile::AYSProjectile()
{
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement");
}
