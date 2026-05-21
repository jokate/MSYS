// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/GameplayCueActor/YSGameplayCueNotify_Sequence.h"

#include "AbilitySystemGlobals.h"
#include "GameplayCueManager.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"


// Sets default values
AYSGameplayCueNotify_Sequence::AYSGameplayCueNotify_Sequence()
{
	PrimaryActorTick.bCanEverTick = false;
}
bool AYSGameplayCueNotify_Sequence::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	Super::OnActive_Implementation(MyTarget, Parameters);
	
	if (SequenceToPlay.IsValid())
	{
		ULevelSequencePlayer* Player = SequenceToPlay->GetSequencePlayer();
		if (ensureMsgf(Player, TEXT("YSGameplayCueNotify_Sequence: SequencePlayer is null")))
		{
			Player->OnStop.AddDynamic(this, &ThisClass::OnSequenceFinished);
			Player->Play();
		}
	}
	
	return true;
}
bool AYSGameplayCueNotify_Sequence::OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	StopSequenceSafely();
	return Super::OnRemove_Implementation(MyTarget, Parameters);
}
void AYSGameplayCueNotify_Sequence::OnSequenceFinished()
{
	if (AActor* OwnerActor = GetOwner())
	{
		if (UGameplayCueManager* CueManager = UAbilitySystemGlobals::Get().GetGameplayCueManager())
		{
			CueManager->NotifyGameplayCueActorFinished(this);
		}
	}
}
void AYSGameplayCueNotify_Sequence::StopSequenceSafely()
{
	if (!SequenceToPlay.IsValid()) { return; }
	ULevelSequencePlayer* Player = SequenceToPlay->GetSequencePlayer();
	if (Player)
	{
		Player->OnStop.RemoveDynamic(this, &ThisClass::OnSequenceFinished);
		Player->Stop();
	}
}