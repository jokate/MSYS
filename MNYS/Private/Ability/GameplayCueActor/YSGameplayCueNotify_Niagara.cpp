// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/GameplayCueActor/YSGameplayCueNotify_Niagara.h"

#include "NiagaraFunctionLibrary.h"


// Sets default values
AYSGameplayCueNotify_Niagara::AYSGameplayCueNotify_Niagara()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

bool AYSGameplayCueNotify_Niagara::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	if (UNiagaraSystem* FX = NiagaraEffect.LoadSynchronous())
	{
		SpawnedFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(	GetWorld(), FX, Parameters.Location, Parameters.Normal.Rotation());
	}
	return Super::OnActive_Implementation(MyTarget, Parameters);
}