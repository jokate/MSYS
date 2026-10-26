// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystem/YSSquadSubsystem.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "YSAbilitySystemComponent.h"
#include "YSDeveloperSettings.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Character/Components/YSCombatTargetComponent.h"
#include "Combat/YSSwitchWindowCloseCondition.h"
#include "Data/YSAbilityDataAsset.h"
#include "Data/YSDataStruct.h"
#include "Engine/World.h"
#include "General/YSGameplayTag.h"


UYSSquadSubsystem::UYSSquadSubsystem()
{
}

void UYSSquadSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	WorldCleanupHandle = FWorldDelegates::OnWorldCleanup.AddUObject(this, &ThisClass::HandleWorldCleanup);
}

void UYSSquadSubsystem::Deinitialize()
{
	FWorldDelegates::OnWorldCleanup.Remove(WorldCleanupHandle);

	Super::Deinitialize();
}

void UYSSquadSubsystem::HandleWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources)
{
	// 남용 기록의 시각은 월드 시계다. 새 월드는 시계가 0 부터라 남겨 두면 미래값이 된다.
	// 태그 게이지는 런 자원이라 방 이동에서 그대로 둔다.
	if (IsValid(World) && World->GetGameInstance() == GetGameInstance())
	{
		ResetSwitchAbuseStates();
	}
}

void UYSSquadSubsystem::InitializeSquad(const TArray<FYSCharacterInfo>& Definitions, const FTransform& SpawnTransform)
{
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	// 게임 인스턴스 수명이라 지난 런 상태가 남아 있다. 새 런은 여기서 비운다.
	Slots.Reset();
	ActiveSlot = 0;
	World->GetTimerManager().ClearTimer(TagCooldownTimer);
	ResetSwitchAbuseStates();

	if (TagGauge != 0.f)
	{
		TagGauge = 0.f;
		OnTagGaugeChanged.Broadcast(TagGauge);
	}

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
	// 태그 교대가 스위치 창을 여는 설정이면 남용 제어도 같은 이유로 교대 전에 본다.
	const UYSDeveloperSettings* Settings = GetDefault<UYSDeveloperSettings>();
	const bool bOpenWindow = bSwitchAttack && Settings->ShouldOpenSwitchWindow(EYSSwitchTrigger::TagSwap);
	float Cost = Settings->SwitchAttackCost;

	if (bOpenWindow && !EvaluateSwitchWindow(EYSSwitchTrigger::TagSwap, GetCurrentCombatTarget(), Settings->SwitchAttackCost, Cost))
	{
		return false;
	}

	if (bSwitchAttack && !TryConsumeGauge(Cost))
	{
		return false;
	}

	PerformTag(SlotIndex, bSwitchAttack, bOpenWindow);
	return true;
}

bool UYSSquadSubsystem::EvaluateSwitchWindow(EYSSwitchTrigger Trigger, const AActor* Target, float BaseCost, float& OutCost) const
{
	OutCost = BaseCost;

	const UYSDeveloperSettings* Settings = GetDefault<UYSDeveloperSettings>();

	if (Settings->ShouldOpenSwitchWindow(Trigger) == false)
	{
		return false;
	}

	// 창 GE 가 없으면 창이 안 열린다. 게이지 · 쿨을 쓰기 전에 거절한다.
	if (Settings->SwitchWindowEffect == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Squad] 스위치 창 GE 가 비어 있어 창을 열지 않았다 (게이지 · 쿨 미소모). YS Developer Settings 에서 채워야 한다."));
		return false;
	}

	const FYSSwitchAbusePolicy* Policy = Settings->ResolveSwitchAbusePolicy(GetActiveASC());

	if (Policy == nullptr)
	{
		return true;
	}

	const UWorld* World = GetWorld();
	const float Now = World ? World->GetTimeSeconds() : 0.f;

	const FYSSwitchAbuseState& AbuseState = GetAbuseState(Trigger);

	if (Policy->CanOpen(AbuseState, Target, Now) == false)
	{
		return false;
	}

	OutCost = Policy->GetGaugeCost(AbuseState, Target, BaseCost);
	return true;
}

bool UYSSquadSubsystem::OpenSwitchWindow(EYSSwitchTrigger Trigger, UYSAbilitySystemComponent* ASC, AActor* Target)
{
	const UWorld* World = GetWorld();
	const TSubclassOf<UGameplayEffect> WindowEffect = GetDefault<UYSDeveloperSettings>()->SwitchWindowEffect;

	// 창이 안 열렸으면 기록도 남기지 않는다.
	if (IsValid(ASC) == false || IsValid(World) == false || WindowEffect == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Squad] 스위치 창을 열지 못했다 — 창 GE 를 걸 ASC 가 없다 (Trigger=%s)."), *UEnum::GetValueAsString(Trigger));
		return false;
	}

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	Context.AddSourceObject(Target);

	const FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(WindowEffect, 1.f, Context);
	const FActiveGameplayEffectHandle Handle = Spec.IsValid() ? ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get()) : FActiveGameplayEffectHandle();

	if (Handle.WasSuccessfullyApplied() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Squad] 스위치 창을 열지 못했다 — 창 GE 가 %s 에 걸리지 않았다 (Trigger=%s)."),
			*GetNameSafe(ASC->GetOwner()), *UEnum::GetValueAsString(Trigger));
		return false;
	}

	const float Now = World->GetTimeSeconds();
	FYSSwitchAbuseState& AbuseState = SwitchAbuseStates.FindOrAdd(GetAbuseStateKey(Trigger));
	AbuseState.Record(Target, Now);
	LastOpenTrigger = Trigger;

	TrackSwitchWindow(Trigger, ASC, Handle, Target);

	float AbuseCooldown = 0.f;
	const FYSSwitchAbusePolicy* Policy = GetDefault<UYSDeveloperSettings>()->ResolveSwitchAbusePolicy(ASC);

	if (Policy != nullptr)
	{
		AbuseCooldown = Policy->GetCooldownRemaining(AbuseState, Target, Now);
	}

	OnSwitchWindowOpened.Broadcast(Trigger, Target, AbuseCooldown);
	return true;
}

const FYSSwitchAbuseState& UYSSquadSubsystem::GetAbuseState(EYSSwitchTrigger Trigger) const
{
	static const FYSSwitchAbuseState Empty;

	const FYSSwitchAbuseState* Found = SwitchAbuseStates.Find(GetAbuseStateKey(Trigger));
	return Found ? *Found : Empty;
}

EYSSwitchTrigger UYSSquadSubsystem::GetAbuseStateKey(EYSSwitchTrigger Trigger) const
{
	return GetDefault<UYSDeveloperSettings>()->SwitchAbuseScope == EYSSwitchAbuseScope::Shared ? EYSSwitchTrigger::TagSwap : Trigger;
}

void UYSSquadSubsystem::ResetSwitchAbuseStates()
{
	SwitchAbuseStates.Reset();
	LastOpenTrigger = EYSSwitchTrigger::TagSwap;

	// 창 기록도 월드 시계 기준이고, 걸려 있던 ASC 는 지난 월드 것이다.
	TrackedSwitchWindows.Reset();
}

void UYSSquadSubsystem::TrackSwitchWindow(EYSSwitchTrigger Trigger, UAbilitySystemComponent* ASC, FActiveGameplayEffectHandle Handle, AActor* Target)
{
	UWorld* World = GetWorld();

	if (IsValid(World) == false || GetDefault<UYSDeveloperSettings>()->FindSwitchWindowCloseCondition(Trigger) == nullptr)
	{
		return;
	}

	// 이전 창의 대상은 이미 화면 중심이 아니라 조건이 참이 될 일이 없다. 남기면 새 창이 일찍 닫혀도 Switch 상태가 상한까지 이어진다.
	for (int32 Index = TrackedSwitchWindows.Num() - 1; Index >= 0; --Index)
	{
		if (TrackedSwitchWindows[Index].Trigger != Trigger)
		{
			continue;
		}

		const FYSTrackedSwitchWindow Previous = TrackedSwitchWindows[Index];
		TrackedSwitchWindows.RemoveAt(Index);

		// 스택형 GE 면 같은 핸들이 돌아온다. 그건 새 창이므로 걷지 않는다.
		if (Previous.Handle != Handle && Previous.ASC.IsValid())
		{
			Previous.ASC->RemoveActiveGameplayEffect(Previous.Handle);
		}
	}

	FYSTrackedSwitchWindow& Tracked = TrackedSwitchWindows.AddDefaulted_GetRef();
	Tracked.Handle = Handle;
	Tracked.ASC = ASC;
	Tracked.Target = Target;
	Tracked.Trigger = Trigger;
	Tracked.OpenTime = World->GetTimeSeconds();

	if (World->GetTimerManager().TimerExists(SwitchWindowPollTimer) == false)
	{
		SwitchWindowPollTimer = World->GetTimerManager().SetTimerForNextTick(this, &ThisClass::PollTrackedSwitchWindows);
	}
}

void UYSSquadSubsystem::PollTrackedSwitchWindows()
{
	UWorld* World = GetWorld();

	if (IsValid(World) == false)
	{
		TrackedSwitchWindows.Reset();
		return;
	}

	const UYSDeveloperSettings* Settings = GetDefault<UYSDeveloperSettings>();
	const float Now = World->GetTimeSeconds();

	for (int32 Index = TrackedSwitchWindows.Num() - 1; Index >= 0; --Index)
	{
		const FYSTrackedSwitchWindow& Tracked = TrackedSwitchWindows[Index];
		UAbilitySystemComponent* ASC = Tracked.ASC.Get();
		const FYSSwitchWindowCloseCondition* Condition = Settings->FindSwitchWindowCloseCondition(Tracked.Trigger);

		if (IsValid(ASC) == false || ASC->GetActiveGameplayEffect(Tracked.Handle) == nullptr || Condition == nullptr)
		{
			TrackedSwitchWindows.RemoveAt(Index);
			continue;
		}

		FYSSwitchWindowCloseContext Context;
		Context.Controller = World->GetFirstPlayerController();
		Context.Pawn = Cast<APawn>(ASC->GetAvatarActor());
		Context.Target = Tracked.Target.Get();
		Context.OpenedFor = Now - Tracked.OpenTime;

		// 기록을 먼저 지운다. GE 제거 콜백 안에서 창이 다시 열리면 배열이 바뀐다.
		if (Condition->CanCloseEarly(Context))
		{
			const FActiveGameplayEffectHandle Handle = Tracked.Handle;
			TrackedSwitchWindows.RemoveAt(Index);
			ASC->RemoveActiveGameplayEffect(Handle);
		}
	}

	if (TrackedSwitchWindows.Num() > 0)
	{
		SwitchWindowPollTimer = World->GetTimerManager().SetTimerForNextTick(this, &ThisClass::PollTrackedSwitchWindows);
	}
}

void UYSSquadSubsystem::CloseSwitchWindow(UAbilitySystemComponent* ASC) const
{
	const TSubclassOf<UGameplayEffect> WindowEffect = GetDefault<UYSDeveloperSettings>()->SwitchWindowEffect;

	if (IsValid(ASC) == false || WindowEffect == nullptr)
	{
		return;
	}

	ASC->RemoveActiveGameplayEffectBySourceEffect(WindowEffect, nullptr);
}

void UYSSquadSubsystem::ReportSwitchAttack(const FYSSwitchAttackReport& Report)
{
	OnSwitchAttackFired.Broadcast(Report);
}

bool UYSSquadSubsystem::TryOpenSwitchWindow(EYSSwitchTrigger Trigger, AActor* Target, float BaseCost)
{
	float Cost = BaseCost;

	if (EvaluateSwitchWindow(Trigger, Target, BaseCost, Cost) == false)
	{
		return false;
	}

	UYSAbilitySystemComponent* ASC = GetActiveASC();

	// 게이지를 쓰기 전에 창을 걸 곳부터 본다.
	if (IsValid(ASC) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Squad] 스위치 창을 열지 못했다 — 필드 캐릭터 ASC 가 없다 (Trigger=%s, 게이지 미소모)."), *UEnum::GetValueAsString(Trigger));
		return false;
	}

	if (TryConsumeGauge(Cost) == false)
	{
		return false;
	}

	if (OpenSwitchWindow(Trigger, ASC, Target) == false)
	{
		AddTagGauge(Cost);
		return false;
	}

	return true;
}

float UYSSquadSubsystem::GetSwitchEffectScale(const AActor* Target) const
{
	const FYSSwitchAbusePolicy* Policy = GetDefault<UYSDeveloperSettings>()->ResolveSwitchAbusePolicy(GetActiveASC());
	const UWorld* World = GetWorld();

	if (Policy == nullptr || World == nullptr)
	{
		return 1.f;
	}

	return Policy->GetEffectScale(GetAbuseState(LastOpenTrigger), Target, World->GetTimeSeconds());
}

float UYSSquadSubsystem::GetSwitchAbuseCooldownRemaining(const AActor* Target) const
{
	const FYSSwitchAbusePolicy* Policy = GetDefault<UYSDeveloperSettings>()->ResolveSwitchAbusePolicy(GetActiveASC());
	const UWorld* World = GetWorld();

	if (Policy == nullptr || World == nullptr)
	{
		return 0.f;
	}

	return Policy->GetCooldownRemaining(GetAbuseState(LastOpenTrigger), Target, World->GetTimeSeconds());
}

AActor* UYSSquadSubsystem::GetCurrentCombatTarget() const
{
	const UWorld* World = GetWorld();
	const APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	const UYSCombatTargetComponent* CombatTarget = UYSCombatTargetComponent::Get(PC);

	return IsValid(CombatTarget) ? CombatTarget->GetCurrentTarget() : nullptr;
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

void UYSSquadSubsystem::PerformTag(int32 NewSlot, bool bSwitchAttack, bool bOpenSwitchWindow)
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
	World->GetTimerManager().SetTimer(TagCooldownTimer, GetDefault<UYSDeveloperSettings>()->TagCooldown, /*bLoop=*/false);
	
	OnActiveSlotChanged.Broadcast(OldSlot, NewSlot);

	// 창을 먼저 연다. 진입기 이벤트가 같은 프레임에 들어온 입력을 Switch 상태로 해석하게 하기 위해서다.
	if (bOpenSwitchWindow)
	{
		OpenSwitchWindow(EYSSwitchTrigger::TagSwap, GetASC(NewSlot), GetCurrentCombatTarget());
	}

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
	TagGauge = FMath::Clamp(TagGauge + Amount, 0.f, GetDefault<UYSDeveloperSettings>()->MaxTagGauge);

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

	TagGauge = FMath::Clamp(TagGauge - Cost, 0.f, GetDefault<UYSDeveloperSettings>()->MaxTagGauge);
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
	if (Slots.Num() > 0)
	{
		return GetASC(ActiveSlot);
	}

	// 스쿼드 없이 띄운 맵(캐릭터 1명)은 조종 중인 폰이 필드 캐릭터다.
	const UWorld* World = GetWorld();
	const APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	APawn* Pawn = IsValid(PC) ? PC->GetPawn() : nullptr;
	UYSAbilitySystemComponent* ASC = IsValid(Pawn) ? Pawn->FindComponentByClass<UYSAbilitySystemComponent>() : nullptr;

	UE_CLOG(IsValid(ASC), LogTemp, Verbose, TEXT("[Squad] 스쿼드가 비어 조종 중인 폰(%s)의 ASC 를 필드 캐릭터로 쓴다."), *GetNameSafe(Pawn));
	return ASC;
}

UYSAbilitySystemComponent* UYSSquadSubsystem::GetASC(int32 SlotIndex) const
{
	if (!Slots.IsValidIndex(SlotIndex) || !IsValid(Slots[SlotIndex].Pawn))
	{
		return nullptr;
	}

	return Slots[SlotIndex].Pawn->FindComponentByClass<UYSAbilitySystemComponent>();
}
