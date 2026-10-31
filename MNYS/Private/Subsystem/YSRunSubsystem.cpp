// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystem/YSRunSubsystem.h"

#include "YSDeveloperSettings.h"
#include "Data/YSDataStruct.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

bool UYSRunSubsystem::StartRun()
{
	const UYSDeveloperSettings* Settings = GetDefault<UYSDeveloperSettings>();
	const UDataTable* Table = Settings->GetStageTable();

	if ( Table == nullptr )
	{
		UE_LOG(LogTemp, Warning, TEXT("[Run] 스테이지 정의 표가 없거나 행 구조가 FYSStageDefinition 이 아니라 런을 시작하지 못했다."));
		return false;
	}

	TArray<FName> Rows = Table->GetRowNames();

	for ( int32 Index = Rows.Num() - 1; Index > 0; --Index )
	{
		Rows.Swap(Index, FMath::RandRange(0, Index));
	}

	StagePlans.Reset();
	ZoneStates.Reset();

	for ( const FName& Row : Rows )
	{
		if ( StagePlans.Num() >= Settings->RunStageCount )
		{
			break;
		}

		const FYSStageDefinition* Definition = Table->FindRow<FYSStageDefinition>(Row, TEXT("StageDefinition"), false);

		if ( Definition == nullptr || Definition->StageMap.IsNull() )
		{
			continue;
		}

		FYSStagePlan& Plan = StagePlans.AddDefaulted_GetRef();
		Plan.StageRow = Row;
		Plan.StageMap = Definition->StageMap;

		for ( const FYSZoneSlotCandidates& Slot : Definition->Slots )
		{
			const int32 Count = Slot.ZoneLevels.Num();
			Plan.SlotZones.Add(Count > 0 ? Slot.ZoneLevels[FMath::RandRange(0, Count - 1)] : TSoftObjectPtr<UWorld>());
		}
	}

	if ( StagePlans.IsEmpty() )
	{
		UE_LOG(LogTemp, Warning, TEXT("[Run] 스테이지 맵이 있는 행이 없어 런을 시작하지 못했다."));
		CurrentStageIndex = INDEX_NONE;
		return false;
	}

	CurrentStageIndex = 0;
	OpenCurrentStage();
	return true;
}

bool UYSRunSubsystem::AdvanceStage()
{
	if ( StagePlans.IsValidIndex(CurrentStageIndex + 1) == false )
	{
		return false;
	}

	++CurrentStageIndex;
	ZoneStates.Reset();
	OpenCurrentStage();
	return true;
}

TSoftObjectPtr<UWorld> UYSRunSubsystem::GetZoneLevel(const UWorld* World, int32 SlotIndex) const
{
	if ( World == nullptr || StagePlans.IsValidIndex(CurrentStageIndex) == false )
	{
		return TSoftObjectPtr<UWorld>();
	}

	const FYSStagePlan& Plan = StagePlans[CurrentStageIndex];

	// PIE 월드는 경로에 UEDPIE_n_ 접두가 붙어 표의 맵 경로와 달라진다.
	if ( FSoftObjectPath(UWorld::RemovePIEPrefix(World->GetPathName())) != Plan.StageMap.ToSoftObjectPath() )
	{
		return TSoftObjectPtr<UWorld>();
	}

	return Plan.SlotZones.IsValidIndex(SlotIndex) ? Plan.SlotZones[SlotIndex] : TSoftObjectPtr<UWorld>();
}

void UYSRunSubsystem::RecordDefeated(int32 SlotIndex, FName ActorName)
{
	ZoneStates.FindOrAdd(SlotIndex).DefeatedActors.Add(ActorName);
}

void UYSRunSubsystem::MarkZoneCleared(int32 SlotIndex)
{
	ZoneStates.FindOrAdd(SlotIndex).bCleared = true;
}

void UYSRunSubsystem::OpenCurrentStage() const
{
	UGameplayStatics::OpenLevelBySoftObjectPtr(this, StagePlans[CurrentStageIndex].StageMap);
}
