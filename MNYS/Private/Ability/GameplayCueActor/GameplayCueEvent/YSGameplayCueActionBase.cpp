// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/GameplayCueActor/GameplayCueEvent/YSGameplayCueActionBase.h"

#include "AbilitySystemGlobals.h"
#include "GameplayCueManager.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "NiagaraFunctionLibrary.h"
#include "Ability/GameplayCueActor/YSGameplayCueNotifyBase.h"
#include "Character/Components/YSCameraManageComponent.h"
#include "Framework/YSGameModeBase.h"

void UYSGameplayCueActionBase::OnActive(AYSGameplayCueNotifyBase* GameplayCueNotify, AActor* MyTarget,
	const FGameplayCueParameters& Parameters)
{
	OwningGameplayCueNotify = GameplayCueNotify; 
}

void UYSGameplayCueAction_SequencePlay::OnActive(AYSGameplayCueNotifyBase* GameplayCueNotify, AActor* MyTarget,
                                                 const FGameplayCueParameters& Parameters)
{
	Super::OnActive(GameplayCueNotify, MyTarget, Parameters);
	if (SequenceToPlay.IsValid())
	{
		ULevelSequencePlayer* Player = SequenceToPlay->GetSequencePlayer();
		if (ensureMsgf(Player, TEXT("YSGameplayCueAction_SequencePlay: SequencePlayer is null")))
		{
			Player->OnStop.AddDynamic(this, &ThisClass::OnSequenceFinished);
			Player->Play();
		}
	}
}

void UYSGameplayCueAction_SequencePlay::OnRemove(AYSGameplayCueNotifyBase* GameplayCueNotify, AActor* MyTarget,
	const FGameplayCueParameters& Parameters)
{
	StopSequenceSafely();
	Super::OnRemove(GameplayCueNotify, MyTarget, Parameters);
}


void UYSGameplayCueAction_SequencePlay::OnSequenceFinished()
{
	if (UGameplayCueManager* CueManager = UAbilitySystemGlobals::Get().GetGameplayCueManager())
	{
		CueManager->NotifyGameplayCueActorFinished(OwningGameplayCueNotify.Get());
	}
}

void UYSGameplayCueAction_SequencePlay::StopSequenceSafely()
{
	if (!SequenceToPlay.IsValid()) { return; }
	ULevelSequencePlayer* Player = SequenceToPlay->GetSequencePlayer();
	if (IsValid(Player) == false)
	{
		Player->OnStop.RemoveDynamic(this, &ThisClass::OnSequenceFinished);
		Player->Stop();
	}
}

void UYSGameplayCueAction_NiagaraEffect::OnActive(AYSGameplayCueNotifyBase* GameplayCueNotify, AActor* MyTarget,
	const FGameplayCueParameters& Parameters)
{
	Super::OnActive(GameplayCueNotify, MyTarget, Parameters);

	if (UNiagaraSystem* FX = NiagaraEffect.LoadSynchronous())
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(	GetWorld(), FX, Parameters.Location, Parameters.Normal.Rotation());
	}
}

void UYSGameplayCueAction_TimeDilation::OnActive(AYSGameplayCueNotifyBase* GameplayCueNotify, AActor* MyTarget,
	const FGameplayCueParameters& Parameters)
{
	Super::OnActive(GameplayCueNotify, MyTarget, Parameters);
	
	UWorld* World = GetWorld();
	
	if ( IsValid(World) == false )
		return;
	
	AYSGameModeBase* GM = World->GetAuthGameMode<AYSGameModeBase>();
	
	if ( IsValid(GM) == false )
	{
		return;
	}
	
	GM->RegisterTimeDilation(this, TimeDilation);
}

void UYSGameplayCueAction_TimeDilation::OnRemove(AYSGameplayCueNotifyBase* GameplayCueNotify, AActor* MyTarget,
	const FGameplayCueParameters& Parameters)
{
	UWorld* World = GetWorld();

	if ( IsValid(World) == false )
		return;

	AYSGameModeBase* GM = World->GetAuthGameMode<AYSGameModeBase>();

	if ( IsValid(GM) == false )
	{
		return;
	}

	GM->RemoveTimeDilation(this);

	Super::OnRemove(GameplayCueNotify, MyTarget, Parameters);
}

void UYSGameplayCueAction_CameraEffect::OnActive(AYSGameplayCueNotifyBase* GameplayCueNotify, AActor* MyTarget,
	const FGameplayCueParameters& Parameters)
{
	Super::OnActive(GameplayCueNotify, MyTarget, Parameters);

	if ( UYSCameraManageComponent* LockOn = UYSCameraManageComponent::Get(MyTarget) )
	{
		LockOn->StartCameraEffect(this, CameraParams);
	}
}

void UYSGameplayCueAction_CameraEffect::OnRemove(AYSGameplayCueNotifyBase* GameplayCueNotify, AActor* MyTarget,
	const FGameplayCueParameters& Parameters)
{
	if ( UYSCameraManageComponent* LockOn = UYSCameraManageComponent::Get(MyTarget) )
	{
		LockOn->StopCameraEffect(this);
	}

	Super::OnRemove(GameplayCueNotify, MyTarget, Parameters);
}
