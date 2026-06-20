// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/GameplayEffectComponents/YSGameplayEffect.h"

#include "NiagaraComponent.h"

void UYSGameplayEffect::AddSpawnedNiagara(UNiagaraComponent* NiagaraComponent)
{
	if ( IsValid(NiagaraComponent) == false )
		return;
		
	SpawnedNiagaraComponent.Add(NiagaraComponent);
}

void UYSGameplayEffect::ResetAllSpawnedNiagara()
{
	for (TWeakObjectPtr<UNiagaraComponent> NiagaraComp : SpawnedNiagaraComponent)
	{
		if (NiagaraComp.IsValid())
		{
			NiagaraComp->DestroyInstance();
		}
	}
	
	SpawnedNiagaraComponent.Empty();
}
