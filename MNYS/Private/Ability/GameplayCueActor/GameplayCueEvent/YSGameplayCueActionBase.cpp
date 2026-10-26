// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/GameplayCueActor/GameplayCueEvent/YSGameplayCueActionBase.h"

#include "AbilitySystemGlobals.h"
#include "GameplayCueManager.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "TimerManager.h"
#include "Ability/GameplayCueActor/YSGameplayCueNotifyBase.h"
#include "Camera/CameraShakeBase.h"
#include "Camera/PlayerCameraManager.h"
#include "Character/Components/YSCameraManageComponent.h"
#include "Components/AudioComponent.h"
#include "Components/MeshComponent.h"
#include "Components/PostProcessComponent.h"
#include "Framework/YSGameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "Input/StateMachine/YSInputStateMachineComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

#pragma region Track
void UYSGameplayCueActionBase::OnActive(AYSGameplayCueNotifyBase* GameplayCueNotify, AActor* MyTarget,
	const FGameplayCueParameters& Parameters)
{
	OwningGameplayCueNotify = GameplayCueNotify;

	if ( bEnabled == false )
		return;

	// 큐 액터가 재활용되면 이전 트랙이 남아 있을 수 있다.
	if ( bBegun )
	{
		HandleEnd();
	}

	ActiveTarget = MyTarget;
	ActiveParameters = Parameters;

	UWorld* World = GetWorld();

	if ( StartTime <= 0.f || IsValid(World) == false )
	{
		HandleBegin();
		return;
	}

	World->GetTimerManager().SetTimer(BeginTimerHandle, this, &ThisClass::HandleBegin, StartTime, false);
}

void UYSGameplayCueActionBase::OnRemove(AYSGameplayCueNotifyBase* GameplayCueNotify, AActor* MyTarget,
	const FGameplayCueParameters& Parameters)
{
	if ( UWorld* World = GetWorld() )
	{
		World->GetTimerManager().ClearTimer(BeginTimerHandle);
	}

	if ( bBegun )
	{
		HandleEnd();
	}

	OwningGameplayCueNotify = nullptr;
}

UWorld* UYSGameplayCueActionBase::GetWorld() const
{
	if ( HasAnyFlags(RF_ClassDefaultObject) )
		return nullptr;

	if ( const AYSGameplayCueNotifyBase* Notify = OwningGameplayCueNotify.Get() )
		return Notify->GetWorld();

	const UObject* Outer = GetOuter();
	return IsValid(Outer) ? Outer->GetWorld() : nullptr;
}

void UYSGameplayCueActionBase::HandleBegin()
{
	bBegun = true;
	Begin(ActiveTarget.Get(), ActiveParameters);

	UWorld* World = GetWorld();

	if ( Duration > 0.f && IsValid(World) )
	{
		World->GetTimerManager().SetTimer(EndTimerHandle, this, &ThisClass::HandleEnd, Duration, false);
	}
}

void UYSGameplayCueActionBase::HandleEnd()
{
	if ( UWorld* World = GetWorld() )
	{
		World->GetTimerManager().ClearTimer(EndTimerHandle);
	}

	if ( bBegun == false )
		return;

	bBegun = false;
	End(ActiveTarget.Get(), ActiveParameters);
}

AActor* UYSGameplayCueActionBase::SelectActor(EYSCueActorSelector Selector, AActor* MyTarget,
	const FGameplayCueParameters& Parameters) const
{
	switch ( Selector )
	{
	case EYSCueActorSelector::Instigator :
		return Parameters.Instigator.Get();
	case EYSCueActorSelector::CueTarget :
	default :
		return MyTarget;
	}
}
#pragma endregion

#pragma region Sequence
void UYSGameplayCueAction_SequencePlay::Begin(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
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

void UYSGameplayCueAction_SequencePlay::End(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	StopSequenceSafely();
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
	if (IsValid(Player))
	{
		Player->OnStop.RemoveDynamic(this, &ThisClass::OnSequenceFinished);
		Player->Stop();
	}
}
#pragma endregion

#pragma region Niagara
void UYSGameplayCueAction_NiagaraEffect::Begin(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	if (UNiagaraSystem* FX = NiagaraEffect.LoadSynchronous())
	{
		if (bNeedToAttach && MyTarget)
		{
			USceneComponent* AttachTo = MyTarget->FindComponentByClass<USkeletalMeshComponent>();
			if (AttachTo == nullptr)
				AttachTo = MyTarget->GetRootComponent();

			SpawnedNiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
				FX, AttachTo, AttachSocketName, AttachmentLocationOffset, AttachmentOffset,
				EAttachLocation::SnapToTarget, true);
		}
		else
		{
			SpawnedNiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), FX, Parameters.Location, Parameters.Normal.Rotation());
		}
	}
}

void UYSGameplayCueAction_NiagaraEffect::End(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	if (SpawnedNiagaraComponent.IsValid())
	{
		SpawnedNiagaraComponent->Deactivate();
		SpawnedNiagaraComponent = nullptr;
	}
}
#pragma endregion

#pragma region TimeDilation
void UYSGameplayCueAction_TimeDilation::Begin(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	UWorld* World = GetWorld();

	if ( IsValid(World) == false )
		return;

	if ( AYSGameModeBase* GM = World->GetAuthGameMode<AYSGameModeBase>() )
	{
		GM->RegisterTimeDilation(this, TimeDilation, Scope, MyTarget);
	}

	if ( bHoldInput )
	{
		if ( UYSInputStateMachineComponent* InputStateMachine = UYSInputStateMachineComponent::Get(MyTarget) )
		{
			InputStateMachine->PushInputHold(this);
		}
	}
}

void UYSGameplayCueAction_TimeDilation::End(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	// 왜곡을 먼저 풀어야 흘린 입력이 평속에서 어빌리티를 켠다.
	UWorld* World = GetWorld();
	AYSGameModeBase* GM = IsValid(World) ? World->GetAuthGameMode<AYSGameModeBase>() : nullptr;

	if ( IsValid(GM) )
	{
		GM->RemoveTimeDilation(this);
	}

	if ( bHoldInput )
	{
		if ( UYSInputStateMachineComponent* InputStateMachine = UYSInputStateMachineComponent::Get(MyTarget) )
		{
			InputStateMachine->PopInputHold(this);
		}
	}
}
#pragma endregion

#pragma region Camera
void UYSGameplayCueAction_CameraEffect::Begin(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	if ( UYSCameraManageComponent* LockOn = UYSCameraManageComponent::Get(SelectActor(CameraOwner, MyTarget, Parameters)) )
	{
		LockOn->PushCameraMode(this, CameraParams, CameraPriority);
		ActiveCameraManage = LockOn;
	}
}

void UYSGameplayCueAction_CameraEffect::End(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	if ( UYSCameraManageComponent* LockOn = ActiveCameraManage.Get() )
	{
		LockOn->StopCameraEffect(this);
	}

	ActiveCameraManage.Reset();
}
#pragma endregion

#pragma region CameraShake
void UYSGameplayCueAction_CameraShake::Begin(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	const AActor* Owner = SelectActor(CameraOwner, MyTarget, Parameters);

	if ( Shake == nullptr || IsValid(Owner) == false )
		return;

	const APlayerController* PC = Owner->GetInstigatorController<APlayerController>();

	if ( IsValid(PC) == false || IsValid(PC->PlayerCameraManager) == false )
		return;

	ActiveCameraManager = PC->PlayerCameraManager;
	ActiveShake = PC->PlayerCameraManager->StartCameraShake(Shake, Scale);
}

void UYSGameplayCueAction_CameraShake::End(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	if ( bStopOnEnd && ActiveCameraManager.IsValid() && ActiveShake.IsValid() )
	{
		ActiveCameraManager->StopCameraShake(ActiveShake.Get());
	}

	ActiveShake.Reset();
	ActiveCameraManager.Reset();
}
#pragma endregion

#pragma region PostProcess
void UYSGameplayCueAction_PostProcess::Begin(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	DestroyComponent();

	UWorld* World = GetWorld();

	if ( IsValid(MyTarget) == false || IsValid(World) == false )
		return;

	UPostProcessComponent* Component = NewObject<UPostProcessComponent>(MyTarget);
	Component->bUnbound = true;
	Component->Priority = Priority;
	Component->Settings = Settings;
	Component->BlendWeight = FadeInTime > 0.f ? 0.f : 1.f;
	Component->RegisterComponent();
	SpawnedComponent = Component;

	if ( FadeInTime > 0.f )
	{
		FadeStartTime = World->GetTimeSeconds();
		FadeStartWeight = 0.f;
		FadeTargetWeight = 1.f;
		FadeLength = FadeInTime;
		World->GetTimerManager().SetTimer(FadeTimerHandle, this, &ThisClass::TickFade, 1.f / 60.f, true);
	}
}

void UYSGameplayCueAction_PostProcess::End(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	UPostProcessComponent* Component = SpawnedComponent.Get();
	UWorld* World = GetWorld();

	if ( IsValid(Component) == false || FadeOutTime <= 0.f || IsValid(World) == false )
	{
		DestroyComponent();
		return;
	}

	FadeStartTime = World->GetTimeSeconds();
	FadeStartWeight = Component->BlendWeight;
	FadeTargetWeight = 0.f;
	FadeLength = FadeOutTime;
	World->GetTimerManager().SetTimer(FadeTimerHandle, this, &ThisClass::TickFade, 1.f / 60.f, true);
}

void UYSGameplayCueAction_PostProcess::TickFade()
{
	UPostProcessComponent* Component = SpawnedComponent.Get();
	UWorld* World = GetWorld();

	if ( IsValid(Component) == false || IsValid(World) == false )
	{
		DestroyComponent();
		return;
	}

	const float Alpha = FMath::Clamp((World->GetTimeSeconds() - FadeStartTime) / FMath::Max(FadeLength, KINDA_SMALL_NUMBER), 0.f, 1.f);
	Component->BlendWeight = FMath::Lerp(FadeStartWeight, FadeTargetWeight, Alpha);

	if ( Alpha < 1.f )
		return;

	World->GetTimerManager().ClearTimer(FadeTimerHandle);

	if ( FadeTargetWeight <= 0.f )
	{
		DestroyComponent();
	}
}

void UYSGameplayCueAction_PostProcess::DestroyComponent()
{
	if ( UWorld* World = GetWorld() )
	{
		World->GetTimerManager().ClearTimer(FadeTimerHandle);
	}

	if ( UPostProcessComponent* Component = SpawnedComponent.Get() )
	{
		Component->DestroyComponent();
	}

	SpawnedComponent = nullptr;
}
#pragma endregion

#pragma region Outline
void UYSGameplayCueAction_Outline::Begin(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	SavedStates.Reset();

	AActor* OutlineActor = SelectActor(Target, MyTarget, Parameters);

	if ( IsValid(OutlineActor) == false )
		return;

	TArray<UMeshComponent*> Meshes;
	OutlineActor->GetComponents<UMeshComponent>(Meshes);

	for ( UMeshComponent* Mesh : Meshes )
	{
		if ( IsValid(Mesh) == false || Mesh->IsVisible() == false )
			continue;

		FSavedDepthState& Saved = SavedStates.AddDefaulted_GetRef();
		Saved.Mesh = Mesh;
		Saved.bRenderCustomDepth = Mesh->bRenderCustomDepth;
		Saved.StencilValue = Mesh->CustomDepthStencilValue;

		Mesh->SetRenderCustomDepth(true);
		Mesh->SetCustomDepthStencilValue(StencilValue);
	}
}

void UYSGameplayCueAction_Outline::End(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	for ( const FSavedDepthState& Saved : SavedStates )
	{
		UMeshComponent* Mesh = Saved.Mesh.Get();

		if ( IsValid(Mesh) == false )
			continue;

		Mesh->SetRenderCustomDepth(Saved.bRenderCustomDepth);
		Mesh->SetCustomDepthStencilValue(Saved.StencilValue);
	}

	SavedStates.Reset();
}
#pragma endregion

#pragma region Sound
void UYSGameplayCueAction_Sound::Begin(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	USoundBase* SoundToPlay = Sound.LoadSynchronous();

	if ( SoundToPlay == nullptr )
		return;

	if ( bPlay2D || IsValid(MyTarget) == false )
	{
		SpawnedAudio = UGameplayStatics::SpawnSound2D(this, SoundToPlay);
		return;
	}

	SpawnedAudio = UGameplayStatics::SpawnSoundAttached(SoundToPlay, MyTarget->GetRootComponent());
}

void UYSGameplayCueAction_Sound::End(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	if ( bStopOnEnd && SpawnedAudio.IsValid() )
	{
		SpawnedAudio->Stop();
	}

	SpawnedAudio = nullptr;
}
#pragma endregion
