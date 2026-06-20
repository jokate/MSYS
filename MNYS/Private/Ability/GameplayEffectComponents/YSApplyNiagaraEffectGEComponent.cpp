// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/GameplayEffectComponents/YSApplyNiagaraEffectGEComponent.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "NiagaraFunctionLibrary.h"
#include "Ability/GameplayEffectComponents/YSGameplayEffect.h"
#include "GameFramework/Character.h"


class UYSGameplayEffect;

void UYSApplyNiagaraEffectGEComponent::OnGameplayEffectApplied(FActiveGameplayEffectsContainer& ActiveGEContainer,
                                                               FGameplayEffectSpec& GESpec, FPredictionKey& PredictionKey) const
{
	UAbilitySystemComponent* ASC = ActiveGEContainer.Owner;
	
	if ( IsValid(ASC) == false || IsValid(TargetToSpawn) == false )
		return;

	AActor* OwnerActor = ASC->GetOwnerActor();

	if ( IsValid(OwnerActor) == false )
	{
		return;
	}
	
	ACharacter* Character = Cast<ACharacter>(OwnerActor);
	
	if ( IsValid(Character) == false)
	{
		return;
	}	
	
	UNiagaraComponent* SpawnedComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(TargetToSpawn, Character->GetMesh(), Socket, 
		FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true);
	
	UYSGameplayEffect* Effect = Cast<UYSGameplayEffect>(GetOwner());
	
	if (IsValid(Effect))
	{
		Effect->AddSpawnedNiagara(SpawnedComponent);
	}
}

void UYSApplyNiagaraEffectGEComponent::OnActiveGameplayEffectRemoved(
	const FGameplayEffectRemovalInfo& GameplayEffectRemovalInfo,
	FActiveGameplayEffectsContainer* ActiveGEContainer) const
{
	UYSGameplayEffect* Effect = Cast<UYSGameplayEffect>(GetOwner());
	
	if (IsValid(Effect))
	{
		Effect->ResetAllSpawnedNiagara();
	}
}
