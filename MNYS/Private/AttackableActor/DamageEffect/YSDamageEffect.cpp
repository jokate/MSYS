// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackableActor/DamageEffect/YSDamageEffect.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "Library/YSBlueprintFunctionLibrary.h"

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
		if (IsValid(TargetASC))
		{
			FGameplayEffectSpecHandle SpecHandle = OwnerASC->MakeOutgoingSpec(GameplayEffectToTarget, Level, EffectContextHandle);
			if (SpecHandle.IsValid())
			{
				OwnerASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
			}
		}
	}

	if ( IsValid(GameplayEffectToInstigator) == false )
	{
		return;
	}

	FGameplayEffectSpecHandle SelfSpecHandle = OwnerASC->MakeOutgoingSpec(GameplayEffectToInstigator, Level, EffectContextHandle);
	if (SelfSpecHandle.IsValid())
	{
		OwnerASC->ApplyGameplayEffectSpecToSelf(*SelfSpecHandle.Data.Get());
	}
}

void FYSDamageEffect_SpawnActor::Apply(const FYSDamageEffectContext& Context) const
{
	if (IsValid(Context.Source) == false)
	{
		return;
	}

	FYSTransformPolicyContext PolicyContext(Context.Source, Context.Target, &Context.HitResult);
	UYSBlueprintFunctionLibrary::SpawnByConfig(Context.Source, Config, PolicyContext, bAttachToTarget ? Context.Target : nullptr, nullptr);
}
