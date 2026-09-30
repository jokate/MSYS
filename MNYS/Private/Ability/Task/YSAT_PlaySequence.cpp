// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/Task/YSAT_PlaySequence.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "DefaultLevelSequenceInstanceData.h"
#include "LevelSequence.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "MovieSceneSequencePlaybackSettings.h"

UYSAT_PlaySequence* UYSAT_PlaySequence::CreatePlaySequenceTask(UGameplayAbility* OwningAbility,
                                                               const FYSSequencePlaySettings& InSettings, AActor* Instigator, AActor* Target)
{
	UYSAT_PlaySequence* Task = NewAbilityTask<UYSAT_PlaySequence>(OwningAbility);
	Task->SequenceSettings = InSettings;
	Task->InstigatorActor = Instigator;
	Task->TargetActor = Target;
	return Task;
}

void UYSAT_PlaySequence::Activate()
{
	Super::Activate();
	
	if ( IsValid(InstigatorActor) == false )
	{
		return;
	}
	
	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InstigatorActor))
	{
		ASC->CurrentMontageStop(SequenceSettings.MontageBlendOutTime);
	}

	ULevelSequence* Sequence = SequenceSettings.Sequence.LoadSynchronous();
	if (!IsValid(Sequence))
	{
		return;
	}
	
	FMovieSceneSequencePlaybackSettings Settings;
	Settings.PlayRate = SequenceSettings.PlayRate;
	Settings.bDisableCameraCuts = !SequenceSettings.bOverrideCameraBySequence;
	// 끝에 닿아도 마지막 프레임을 쥔 채 OnFinished 만 쏜다.
	// 포즈·카메라 복원은 다음 노드가 시작되는 EndPlay 의 Stop 에서 일어나므로 노드 경계에서 AnimBP 포즈가 새지 않는다.
	ALevelSequenceActor* SequenceActor = nullptr;
	LevelSequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer( this, Sequence , Settings, SequenceActor);
	
	if (!IsValid(LevelSequencePlayer) || !IsValid(SequenceActor))
	{
		return;
	}
	
	LevelSequenceActor = SequenceActor;

	// 시퀀스는 시전자 로컬 공간(원점 = 시전자 위치, +X = 정면)으로 저작한다.
	// 어태치 대신 Transform Origin 을 쓰는 이유: 어태치는 플레이어가 움직이면 적·카메라가 같이 끌려가고, CMC 와도 충돌한다.
	LevelSequenceActor->bOverrideInstanceData = true;
	if (UDefaultLevelSequenceInstanceData* InstanceData = Cast<UDefaultLevelSequenceInstanceData>(LevelSequenceActor->DefaultInstanceData))
	{
		InstanceData->TransformOrigin = FTransform(InstigatorActor->GetActorRotation(), InstigatorActor->GetActorLocation());
	}

	LevelSequenceActor->SetBindingByTag(TEXT("Player"), { InstigatorActor });
	
	if (LevelSequenceActor->FindNamedBinding(TEXT("Enemy")).IsValid())	
	{
		if ( IsValid(TargetActor) )
		{
			UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
			
			if (IsValid(TargetASC) )
			{
				TargetASC->CurrentMontageStop(SequenceSettings.MontageBlendOutTime);
			}
			
        	LevelSequenceActor->SetBindingByTag(TEXT("Enemy"), { TargetActor });	
		}
	}
	
	LevelSequencePlayer->OnFinished.AddDynamic(this, &ThisClass::OnSequenceFinished);
	LevelSequencePlayer->Play();
}

void UYSAT_PlaySequence::OnDestroy(bool bInOwnerFinished)
{
	if ( IsValid(LevelSequencePlayer) )
	{
		LevelSequencePlayer->OnFinished.RemoveAll(this);
	}

	if ( IsValid(LevelSequenceActor) )
	{
		LevelSequenceActor->SetLifeSpan(0.01f);
	}

	LevelSequencePlayer = nullptr;
	LevelSequenceActor = nullptr;

	Super::OnDestroy(bInOwnerFinished);
}

void UYSAT_PlaySequence::OnSequenceFinished()
{
	if ( ShouldBroadcastAbilityTaskDelegates() )
	{
		OnSequenceFinishedDelegate.Broadcast();
	}
}

