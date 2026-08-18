// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackableActor/DamageEffect/YSDamageEffect.h"

#include "GameFramework/Character.h"

void FYSDamageEffect_Knockback::Apply(const FYSDamageEffectContext& Context) const
{
	ACharacter* TargetCharacter = Cast<ACharacter>(Context.Target);

	if ( IsValid(TargetCharacter) == false )
	{
		return;
	}
	
	const AActor* Origin = IsValid(Context.Source) ? Context.Source.Get() : Context.Instigator.Get();

	if ( IsValid(Origin) == false )
	{
		return;
	}
	
	FVector Direction = ( bUseImpactNormal )
		? -Context.HitResult.ImpactNormal
		: TargetCharacter->GetActorLocation() - Origin->GetActorLocation();

	Direction.Z = 0.f;

	// 정확히 겹쳐 서 있거나 노멀이 수직인 경우.
	if ( Direction.IsNearlyZero() )
	{
		Direction = Origin->GetActorForwardVector();
		Direction.Z = 0.f;
	}

	TargetCharacter->LaunchCharacter(Direction.GetSafeNormal() * Strength + FVector::UpVector * Lift, true, true);
}
