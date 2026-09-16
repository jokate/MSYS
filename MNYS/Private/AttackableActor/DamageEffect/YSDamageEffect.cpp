// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackableActor/DamageEffect/YSDamageEffect.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
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

void FYSDamageEffect_ApplyGameplayEffect::Apply(const FYSDamageEffectContext& Context) const
{
	if ( GameplayEffectToTarget == nullptr && GameplayEffectToInstigator == nullptr )
	{
		return;
	}
	
	AActor* Instigator = Context.Instigator;
	if (IsValid(Instigator) == false)
	{
		return;
	}
	
	UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Instigator);
	
	if ( IsValid(OwnerASC) == false )
	{
		return;
	}
	
	FGameplayEffectContextHandle EffectContextHandle = OwnerASC->MakeEffectContext();
	EffectContextHandle.AddInstigator(Instigator, Instigator);
	
	if (IsValid(Context.Target) && IsValid(GameplayEffectToTarget))
	{
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Context.Target);	
		FGameplayEffectSpec Spec(GameplayEffectToTarget.GetDefaultObject(), EffectContextHandle);	
		OwnerASC->ApplyGameplayEffectSpecToTarget(Spec, TargetASC);
	}
	
	if ( IsValid(GameplayEffectToInstigator) == false )
	{
		return;
	}
	
	FGameplayEffectSpec EffectSpec(GameplayEffectToInstigator.GetDefaultObject(), EffectContextHandle);	
	OwnerASC->ApplyGameplayEffectSpecToSelf(EffectSpec);
}
