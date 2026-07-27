// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystem/YSSquadSubsystem.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "YSAbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Data/YSAbilityDataAsset.h"
#include "Data/YSDataStruct.h"
#include "General/YSGameplayTag.h"


UYSSquadSubsystem::UYSSquadSubsystem()
{
}

void UYSSquadSubsystem::InitializeSquad(const TArray<FYSCharacterInfo>& Definitions, const FTransform& SpawnTransform)
{
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	Slots.Reset();

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	for (const FYSCharacterInfo& Definition : Definitions)
	{
		if (IsValid(Definition.PawnClass) == false )
		{
			continue;
		}

		FYSSquadSlot Slot;
		Slot.CharacterInfo = Definition;
		Slot.Pawn = World->SpawnActor<APawn>(Definition.PawnClass, SpawnTransform, Params);

		if (Slot.Pawn)
		{
			Slots.Add(MoveTemp(Slot));
		}
	}

	if (Slots.Num() == 0)
	{
		return;
	}

	ActiveSlot = 0;

	// 0번만 활성, 나머지는 벤치
	for (int32 Index = 0; Index < Slots.Num(); ++Index)
	{
		SetPawnActive(Slots[Index].Pawn, Index == ActiveSlot);
	}

	// 컨트롤러가 0번을 소유하게 한다
	if (APlayerController* PC = Cast<APlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		if (APawn* Target = Slots[ActiveSlot].Pawn)
		{
			PC->Possess(Target);
			PC->SetViewTarget(Target);
		}
	}

	RefreshBackupPassives();
	OnActiveSlotChanged.Broadcast(INDEX_NONE, ActiveSlot);
}

bool UYSSquadSubsystem::RequestTag(int32 SlotIndex, bool bSwitchAttack)
{
	if (!Slots.IsValidIndex(SlotIndex) || SlotIndex == ActiveSlot)
	{
		return false;
	}

	if (IsTagOnCooldown())
	{
		return false;
	}

	if (!IsValid(Slots[SlotIndex].Pawn))
	{
		return false;
	}

	// 스위치 어택은 게이지를 먼저 확보한다. 실패하면 일반 교대로 강등하지 않고 그냥 거절한다
	// (플레이어가 진입기를 기대하고 눌렀는데 맨몸으로 들어가면 그대로 맞는다).
	if (bSwitchAttack && !TryConsumeGauge(SwitchAttackCost))
	{
		return false;
	}

	PerformTag(SlotIndex, bSwitchAttack);
	return true;
}

bool UYSSquadSubsystem::ForceTagOnDown()
{
	for (int32 Index = 0; Index < Slots.Num(); ++Index)
	{
		if (Index == ActiveSlot || !IsValid(Slots[Index].Pawn))
		{
			continue;
		}

		// 다운 강제 교대는 쿨다운을 무시한다
		PerformTag(Index, false);
		return true;
	}
	
	return false;
}

void UYSSquadSubsystem::PerformTag(int32 NewSlot, bool bSwitchAttack)
{
	UWorld* World = GetWorld();
	if (!IsValid(World) || !Slots.IsValidIndex(NewSlot))
	{
		return;
	}

	const int32 OldSlot = ActiveSlot;
	APawn* OldPawn = Slots.IsValidIndex(OldSlot) ? Slots[OldSlot].Pawn : nullptr;
	APawn* NewPawn = Slots[NewSlot].Pawn;

	if (!IsValid(NewPawn))
	{
		return;
	}

	// 새 캐릭터는 이전 캐릭터의 위치를 이어받는다
	if (IsValid(OldPawn))
	{
		NewPawn->SetActorTransform(OldPawn->GetActorTransform(), false, nullptr, ETeleportType::TeleportPhysics);
	}

	ActiveSlot = NewSlot;

	SetPawnActive(NewPawn, true);
	SetPawnActive(OldPawn, false);

	// 소유권 이전 — 카메라는 짧게 블렌드해야 교대가 뚝 끊기지 않는다
	if (APlayerController* PC = Cast<APlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		PC->UnPossess();
		PC->Possess(NewPawn);
		PC->SetViewTargetWithBlend(NewPawn, 0.15f);
	}

	RefreshBackupPassives();

	// 교대 쿨다운
	World->GetTimerManager().SetTimer(TagCooldownTimer, TagCooldown, /*bLoop=*/false);
	
	OnActiveSlotChanged.Broadcast(OldSlot, NewSlot);

	// 스위치 어택 — 진입기 발동
	if (bSwitchAttack)
	{
		FGameplayEventData EventData;
		EventData.EventTag = YSTags::Event_TagEnter;
		EventData.Instigator = NewPawn;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(NewPawn, YSTags::Event_TagEnter, EventData);
	}
}

void UYSSquadSubsystem::RefreshBackupPassives()
{
	UYSAbilitySystemComponent* FieldASC = GetASC(ActiveSlot);
	if (IsValid(FieldASC) == false )
	{
		return;
	}

	UYSAbilityDataAsset* DataAsset = UYSAbilityDataAsset::GetDataAssetFromAbilitySystemComponent(FieldASC);
	if (IsValid(DataAsset) == false)
	{
		return;
	}
	
	for (int32 Index = 0; Index < Slots.Num(); ++Index)
	{
		FYSSquadSlot& Slot = Slots[Index];

		// 이전에 붙여둔 백업 패시브는 대상이 바뀌었으므로 전부 제거한다.
		// 핸들이 가리키는 ASC가 이미 교체됐을 수 있어 각 슬롯 ASC에서 개별 제거한다.
		if (Slot.BackupHandleOnField.IsValid())
		{
			for (int32 Inner = 0; Inner < Slots.Num(); ++Inner)
			{
				if (UYSAbilitySystemComponent* ASC = GetASC(Inner))
				{
					ASC->RemoveActiveGameplayEffect(Slot.BackupHandleOnField);
				}
			}
			Slot.BackupHandleOnField.Invalidate();
		}
		

		// 벤치인 슬롯만 백업 패시브를 필드에 제공한다
		if (Index == ActiveSlot)
		{
			continue;
		}

		if (TSubclassOf<UGameplayEffect> BackupPassiveEffect = DataAsset->GetBackupPassiveEffect())
		{
			FGameplayEffectContextHandle Context = FieldASC->MakeEffectContext();
			Context.AddSourceObject(FieldASC);

			const FGameplayEffectSpecHandle Spec =
				FieldASC->MakeOutgoingSpec(BackupPassiveEffect, 1.f, Context);

			if (Spec.IsValid())
			{
				Slot.BackupHandleOnField = FieldASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
			}
		}
	}
}

void UYSSquadSubsystem::SetPawnActive(APawn* Pawn, bool bActive)
{
	if (!IsValid(Pawn))
	{
		return;
	}

	Pawn->SetActorHiddenInGame(!bActive);
	Pawn->SetActorEnableCollision(bActive);
	
	Pawn->SetActorTickEnabled(bActive);
}

bool UYSSquadSubsystem::IsTagOnCooldown() const
{
	const UWorld* World = GetWorld();
	return World && World->GetTimerManager().IsTimerActive(TagCooldownTimer);
}

APawn* UYSSquadSubsystem::GetActivePawn() const
{
	return Slots.IsValidIndex(ActiveSlot) ? Slots[ActiveSlot].Pawn : nullptr;
}

void UYSSquadSubsystem::AddTagGauge(float Amount)
{
	if (Amount <= 0.f)
	{
		return;
	}

	const float Old = TagGauge;
	TagGauge = FMath::Clamp(TagGauge + Amount, 0.f, MaxTagGauge);

	if (!FMath::IsNearlyEqual(Old, TagGauge))
	{
		OnTagGaugeChanged.Broadcast(TagGauge);
	}
}

bool UYSSquadSubsystem::TryConsumeGauge(float Cost)
{
	if (Cost <= 0.f)
	{
		return true;
	}

	if (TagGauge + KINDA_SMALL_NUMBER < Cost)
	{
		return false;
	}

	TagGauge = FMath::Clamp(TagGauge - Cost, 0.f, MaxTagGauge);
	OnTagGaugeChanged.Broadcast(TagGauge);
	return true;
}

bool UYSSquadSubsystem::GetSlotInfo(int32 SlotIndex, FYSCharacterInfo& OutDefinition, UYSAbilitySystemComponent*& OutASC) const
{
	OutASC = nullptr;

	if (!Slots.IsValidIndex(SlotIndex))
	{
		return false;
	}

	OutDefinition = Slots[SlotIndex].CharacterInfo;
	OutASC = GetASC(SlotIndex);
	return true;
}

UYSAbilitySystemComponent* UYSSquadSubsystem::GetActiveASC() const
{
	return GetASC(ActiveSlot);
}

UYSAbilitySystemComponent* UYSSquadSubsystem::GetASC(int32 SlotIndex) const
{
	if (!Slots.IsValidIndex(SlotIndex) || !IsValid(Slots[SlotIndex].Pawn))
	{
		return nullptr;
	}

	return Slots[SlotIndex].Pawn->FindComponentByClass<UYSAbilitySystemComponent>();
}
