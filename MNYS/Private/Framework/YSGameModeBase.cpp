// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/YSGameModeBase.h"

#include "EngineUtils.h"
#include "GenericTeamAgentInterface.h"
#include "YSDeveloperSettings.h"
#include "Engine/Level.h"
#include "Engine/LevelStreamingDynamic.h"
#include "GameFramework/PlayerController.h"
#include "Stage/YSZoneSlot.h"
#include "Subsystem/YSRunSubsystem.h"

namespace
{
	// CustomTimeDilation 이 0 이면 이동 계산에서 0 으로 나누는 경로가 생긴다. 멈춤은 이 값으로 대신한다.
	constexpr float MinTimeDilation = 0.0001f;
}

bool FYSTimeDilationFactor::AppliesTo(const AActor* Actor) const
{
	if ( Scope == EYSTimeDilationScope::All )
	{
		return true;
	}

	const AActor* Owner = ScopeOwner.Get();

	if ( Scope == EYSTimeDilationScope::Participants )
	{
		return Actor == Owner || ScopeActors.Contains(Actor);
	}

	if ( IsValid(Owner) == false || Actor == Owner )
	{
		return false;
	}

	const IGenericTeamAgentInterface* TeamAgent = Cast<const IGenericTeamAgentInterface>(Owner);

	if ( TeamAgent == nullptr )
	{
		return false;
	}

	return TeamAgent->GetTeamAttitudeTowards(*Actor) != ETeamAttitude::Friendly;
}

void AYSGameModeBase::SetTimeDilation()
{
	for (TWeakObjectPtr<AActor> TimeDilationTarget : TimeDilationActors )
	{
		AActor* TimeDilationActor = TimeDilationTarget.Get();

		if ( IsValid(TimeDilationActor) == false )
		{
			continue;
		}

		// 액터마다 자기에게 해당하는 가장 최근 요청을 쓴다.
		float Dilation = 1.f;

		for ( int32 Index = TimeDilationFactors.Num() - 1; Index >= 0; --Index )
		{
			if ( TimeDilationFactors[Index].AppliesTo(TimeDilationActor) )
			{
				Dilation = TimeDilationFactors[Index].TimeDilation;
				break;
			}
		}

		TimeDilationActor->CustomTimeDilation = FMath::Max(Dilation, MinTimeDilation);
	}
}

void AYSGameModeBase::RegisterTimeDilation(UObject* Object, float TimeDilation, EYSTimeDilationScope Scope, AActor* ScopeOwner, const TArray<AActor*>& ScopeActors)
{
	if ( Object == nullptr )
		return;

	FYSTimeDilationFactor NewFactor;
	NewFactor.DilationCaller = Object;
	NewFactor.TimeDilation = TimeDilation;
	NewFactor.Scope = Scope;
	NewFactor.ScopeOwner = ScopeOwner;

	for ( AActor* ScopeActor : ScopeActors )
	{
		NewFactor.ScopeActors.Add(ScopeActor);
	}

	TimeDilationFactors.Add(NewFactor);

	SetTimeDilation();
}

void AYSGameModeBase::RemoveTimeDilation(UObject* Object)
{
	if ( Object == nullptr )
		return;

	TimeDilationFactors.RemoveAll([Object](const FYSTimeDilationFactor& Factor) { return Factor == Object; });

	SetTimeDilation();
}

void AYSGameModeBase::StartTimedDilation(float TimeDilation, float Duration, EYSTimeDilationScope Scope, AActor* ScopeOwner, const TArray<AActor*>& ScopeActors)
{
	UWorld* World = GetWorld();

	if ( Duration <= 0.f || IsValid(World) == false )
	{
		return;
	}

	// 요청마다 키가 달라야 한 요청의 해제가 다른 요청을 걷어내지 않는다. 팩터가 강참조로 붙들어 해제 전에는 사라지지 않는다.
	UObject* Caller = NewObject<UObject>(this);
	RegisterTimeDilation(Caller, TimeDilation, Scope, ScopeOwner, ScopeActors);

	// 월드 타이머는 액터 시간 배율을 받지 않아 정지 중에도 제때 풀린다.
	FTimerHandle Handle;
	World->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateWeakLambda(this, [this, WeakCaller = TWeakObjectPtr<UObject>(Caller)]()
	{
		RemoveTimeDilation(WeakCaller.Get());
	}), Duration, false);
}

void AYSGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	for ( TActorIterator<AYSZoneSlot> It(GetWorld()); It; ++It )
	{
		ZoneSlots.Add(*It);
	}
}

void AYSGameModeBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	SetTimeDilation();
	UpdateZoneStreaming();
}

void AYSGameModeBase::RecordZoneActorDefeated(AActor* Actor)
{
	UYSRunSubsystem* Run = GetGameInstance() ? GetGameInstance()->GetSubsystem<UYSRunSubsystem>() : nullptr;

	if ( IsValid(Actor) == false || Run == nullptr )
	{
		return;
	}

	const ULevel* Level = Actor->GetLevel();

	for ( const TPair<int32, FYSLoadedZone>& Pair : LoadedZones )
	{
		if ( Pair.Value.Streaming != nullptr && Pair.Value.Streaming->GetLoadedLevel() == Level )
		{
			Run->RecordDefeated(Pair.Key, Actor->GetFName());
			return;
		}
	}
}

void AYSGameModeBase::UpdateZoneStreaming()
{
	UWorld* World = GetWorld();
	const APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	const APawn* Pawn = PC ? PC->GetPawn() : nullptr;

	if ( ZoneSlots.IsEmpty() || IsValid(Pawn) == false )
	{
		return;
	}

	const UYSRunSubsystem* Run = GetGameInstance() ? GetGameInstance()->GetSubsystem<UYSRunSubsystem>() : nullptr;
	const UYSDeveloperSettings* Settings = GetDefault<UYSDeveloperSettings>();
	const FVector PlayerLocation = Pawn->GetActorLocation();

	for ( const TWeakObjectPtr<AYSZoneSlot>& WeakSlot : ZoneSlots )
	{
		const AYSZoneSlot* ZoneSlot = WeakSlot.Get();

		if ( ZoneSlot == nullptr )
		{
			continue;
		}

		const int32 SlotIndex = ZoneSlot->GetSlotIndex();
		const float Distance = ZoneSlot->GetDistanceTo(PlayerLocation);
		FYSLoadedZone* Loaded = LoadedZones.Find(SlotIndex);

		if ( Loaded == nullptr )
		{
			const TSoftObjectPtr<UWorld> Zone = Run ? Run->GetZoneLevel(World, SlotIndex) : TSoftObjectPtr<UWorld>();

			if ( Distance > Settings->ZoneLoadDistance || Zone.IsNull() )
			{
				continue;
			}

			bool bSuccess = false;
			const ULevelStreamingDynamic::FLoadLevelInstanceParams Params(World, Zone.ToString(), ZoneSlot->GetActorTransform());
			ULevelStreamingDynamic* Streaming = ULevelStreamingDynamic::LoadLevelInstance(Params, bSuccess);

			if ( bSuccess == false )
			{
				UE_LOG(LogTemp, Warning, TEXT("[Stage] 자리 %d 에 구역 %s 을 띄우지 못했다."), SlotIndex, *Zone.ToString());
			}

			FYSLoadedZone& NewZone = LoadedZones.Add(SlotIndex);
			NewZone.Streaming = bSuccess ? Streaming : nullptr;
			continue;
		}

		if ( Distance > Settings->ZoneUnloadDistance )
		{
			if ( Loaded->Streaming != nullptr )
			{
				Loaded->Streaming->SetIsRequestingUnloadAndRemoval(true);
			}

			LoadedZones.Remove(SlotIndex);
			continue;
		}

		if ( Loaded->bStateApplied == false && Loaded->Streaming != nullptr && Loaded->Streaming->IsLevelVisible() )
		{
			ApplyZoneState(SlotIndex, Loaded->Streaming->GetLoadedLevel());
			Loaded->bStateApplied = true;
		}
	}
}

void AYSGameModeBase::ApplyZoneState(int32 SlotIndex, const ULevel* Level) const
{
	const UYSRunSubsystem* Run = GetGameInstance() ? GetGameInstance()->GetSubsystem<UYSRunSubsystem>() : nullptr;
	const FYSZoneState* State = Run ? Run->FindZoneState(SlotIndex) : nullptr;

	if ( State == nullptr || Level == nullptr )
	{
		return;
	}

	// 지우는 동안 레벨의 액터 배열이 바뀌지 않게 먼저 모은다.
	TArray<AActor*> Defeated;

	for ( AActor* Actor : Level->Actors )
	{
		if ( IsValid(Actor) && State->DefeatedActors.Contains(Actor->GetFName()) )
		{
			Defeated.Add(Actor);
		}
	}

	for ( AActor* Actor : Defeated )
	{
		Actor->Destroy();
	}
}
