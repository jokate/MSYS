// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/YSGroggyComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "TimerManager.h"
#include "YSDeveloperSettings.h"
#include "Ability/GameplayEffect/YSGE_GroggyBreak.h"
#include "Animation/AnimMontage.h"
#include "Character/AttributeSet/YSGroggyAttributeSet.h"
#include "Combat/YSGroggyRules.h"
#include "GameFramework/Character.h"
#include "General/YSGameplayTag.h"
#include "Subsystem/YSCombatEventSubsystem.h"

UYSGroggyComponent::UYSGroggyComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UYSGroggyComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	CachedASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);

	if ( UAbilitySystemComponent* ASC = CachedASC.Get() )
	{
		BrokenEventHandle = ASC->GenericGameplayEventCallbacks.FindOrAdd(YSTags::Event_GroggyBroken)
			.AddUObject(this, &ThisClass::OnGroggyBroken);
	}

	if ( UYSCombatEventSubsystem* CombatEvents = UYSCombatEventSubsystem::Get(this) )
	{
		PhaseChangedHandle = CombatEvents->OnPhaseChanged.AddUObject(this, &ThisClass::OnPhaseChanged);
	}
}

void UYSGroggyComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearPhases();

	if ( UAbilitySystemComponent* ASC = CachedASC.Get() )
	{
		if ( FGameplayEventMulticastDelegate* Delegate = ASC->GenericGameplayEventCallbacks.Find(YSTags::Event_GroggyBroken) )
		{
			Delegate->Remove(BrokenEventHandle);
		}
	}

	if ( UYSCombatEventSubsystem* CombatEvents = UYSCombatEventSubsystem::Get(this) )
	{
		CombatEvents->OnPhaseChanged.Remove(PhaseChangedHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void UYSGroggyComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if ( IsBroken() == false )
	{
		Regenerate(DeltaTime);
	}
}

const FYSGroggyRules& UYSGroggyComponent::GetRules() const
{
	return IsValid(Profile) ? Profile->Rules : GetDefault<UYSDeveloperSettings>()->DefaultGroggyRules;
}

const UYSGroggyAttributeSet* UYSGroggyComponent::GetGroggySet() const
{
	const UAbilitySystemComponent* ASC = CachedASC.Get();
	return ASC != nullptr ? ASC->GetSet<UYSGroggyAttributeSet>() : nullptr;
}

void UYSGroggyComponent::OnGroggyBroken(const FGameplayEventData* Payload)
{
	if ( IsBroken() || CachedASC.IsValid() == false )
	{
		return;
	}

	const FYSGroggyRules& Rules = GetRules();

	ClearPhases();
	BreakInstigator = Payload != nullptr ? const_cast<AActor*>(Payload->Instigator.Get()) : nullptr;

	CaptureBaseMaxGroggy();
	CancelActiveAbilities(Rules);
	ApplyBreakEffect(Rules);

	if ( IsBroken() == false )
	{
		return;
	}

	PlayBreakMontage(Rules);

	// 무방비는 진입 정지를 감싼다: 0 ~ 정지 + 브레이크 시간. 예고는 끝나기 WarningLeadTime 전부터.
	const float TotalDuration = Rules.EntryHoldTime + Rules.BreakDuration;

	BeginPhase(EYSBreakPhase::Broken);
	BeginPhase(EYSBreakPhase::Enter);
	SchedulePhase(EnterEndTimer, EYSBreakPhase::Enter, false, Rules.EntryHoldTime);

	if ( Rules.WarningLeadTime > 0.f )
	{
		SchedulePhase(WarningTimer, EYSBreakPhase::Warning, true, FMath::Max(TotalDuration - Rules.WarningLeadTime, Rules.EntryHoldTime));
	}
}

void UYSGroggyComponent::CancelActiveAbilities(const FYSGroggyRules& Rules)
{
	FGameplayTagContainer KeepTags;

	for ( const FGameplayTag& Tag : Rules.KeepAbilitiesWithTags )
	{
		KeepTags.AddTag(Tag);
	}

	CachedASC->CancelAbilities(nullptr, &KeepTags);
}

void UYSGroggyComponent::ApplyBreakEffect(const FYSGroggyRules& Rules)
{
	UAbilitySystemComponent* ASC = CachedASC.Get();
	TSubclassOf<UGameplayEffect> EffectClass = GetDefault<UYSDeveloperSettings>()->GroggyBreakEffect;

	if ( EffectClass == nullptr )
	{
		EffectClass = UYSGE_GroggyBreak::StaticClass();
	}

	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(EffectClass, 1.f, ASC->MakeEffectContext());

	if ( SpecHandle.IsValid() == false )
	{
		return;
	}

	// GE 시간은 월드 시간으로 흘러 진입 정지 중에도 줄어든다. 브레이크 시간을 정지 뒤부터 세도록 정지만큼 늘린다.
	const float TotalDuration = Rules.EntryHoldTime + Rules.BreakDuration;

	FGameplayEffectSpec* Spec = SpecHandle.Data.Get();
	Spec->SetSetByCallerMagnitude(YSTags::Data_Groggy_BreakDuration, TotalDuration);
	Spec->SetSetByCallerMagnitude(YSTags::Data_Groggy_DamageTakenMultiplier, Rules.DamageTakenMultiplier);
	// SetByCaller 로 정한 시간은 스펙 생성 때 계산되지 않아 다시 잡는다.
	Spec->SetDuration(TotalDuration, true);

	Spec->DynamicGrantedTags.AddTag(YSTags::State_Groggy_Broken);

	for ( const FGameplayTag& Tag : Rules.GrantedTags )
	{
		Spec->DynamicGrantedTags.AddTag(Tag);
	}

	BreakHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec);

	if ( FOnActiveGameplayEffectRemoved_Info* Removed = ASC->OnGameplayEffectRemoved_InfoDelegate(BreakHandle) )
	{
		Removed->AddUObject(this, &ThisClass::OnBreakEffectRemoved);
	}
}

void UYSGroggyComponent::PlayBreakMontage(const FYSGroggyRules& Rules)
{
	ACharacter* Character = GetOwner<ACharacter>();
	UAnimMontage* Montage = Rules.BreakMontage.LoadSynchronous();

	if ( IsValid(Character) == false || IsValid(Montage) == false )
	{
		return;
	}

	Character->PlayAnimMontage(Montage);
	PlayingMontage = Montage;
}

void UYSGroggyComponent::StopBreakMontage()
{
	ACharacter* Character = GetOwner<ACharacter>();

	if ( IsValid(Character) && PlayingMontage.IsValid() )
	{
		Character->StopAnimMontage(PlayingMontage.Get());
	}

	PlayingMontage.Reset();
}

void UYSGroggyComponent::OnBreakEffectRemoved(const FGameplayEffectRemovalInfo& RemovalInfo)
{
	BreakHandle.Invalidate();
	StopBreakMontage();
	ClearPhases();

	++BreakCount;
	ApplyMaxGroggy();

	UYSGroggyAttributeSet::RestoreGroggy(GetOwner(), 1.f);

	const float Grace = GetRules().PostBreakGrace;

	if ( const UYSGroggyAttributeSet* GroggySet = GetGroggySet() )
	{
		const_cast<UYSGroggyAttributeSet*>(GroggySet)->LockDamage(Grace);
	}

	if ( Grace > 0.f )
	{
		BeginPhase(EYSBreakPhase::Grace);
		SchedulePhase(GraceEndTimer, EYSBreakPhase::Grace, false, Grace);
	}
}

void UYSGroggyComponent::BeginPhase(EYSBreakPhase Phase)
{
	UAbilitySystemComponent* ASC = CachedASC.Get();

	if ( ASC == nullptr || ActivePhases.Contains(Phase) )
	{
		return;
	}

	ActivePhases.Add(Phase);

	// YS Cue 액터는 Add/Remove 로만 트랙을 돌린다 (Execute 는 받지 않는다).
	const FGameplayTag* CueTag = GetRules().PhaseCues.Find(Phase);

	if ( CueTag != nullptr && CueTag->IsValid() )
	{
		FGameplayCueParameters Parameters;
		Parameters.Instigator = BreakInstigator;
		Parameters.EffectCauser = BreakInstigator;
		ASC->AddGameplayCue(*CueTag, Parameters);
	}

	OnBreakPhaseChanged.Broadcast(Phase, true);
}

void UYSGroggyComponent::EndPhase(EYSBreakPhase Phase)
{
	if ( ActivePhases.Remove(Phase) == 0 )
	{
		return;
	}

	const FGameplayTag* CueTag = GetRules().PhaseCues.Find(Phase);
	UAbilitySystemComponent* ASC = CachedASC.Get();

	if ( ASC != nullptr && CueTag != nullptr && CueTag->IsValid() )
	{
		ASC->RemoveGameplayCue(*CueTag);
	}

	OnBreakPhaseChanged.Broadcast(Phase, false);
}

void UYSGroggyComponent::SchedulePhase(FTimerHandle& Handle, EYSBreakPhase Phase, bool bBegin, float Delay)
{
	UWorld* World = GetWorld();

	if ( World == nullptr )
	{
		return;
	}

	if ( Delay <= 0.f )
	{
		bBegin ? BeginPhase(Phase) : EndPhase(Phase);
		return;
	}

	const FTimerDelegate Delegate = bBegin
		? FTimerDelegate::CreateUObject(this, &ThisClass::BeginPhase, Phase)
		: FTimerDelegate::CreateUObject(this, &ThisClass::EndPhase, Phase);

	World->GetTimerManager().SetTimer(Handle, Delegate, Delay, false);
}

void UYSGroggyComponent::ClearPhases()
{
	if ( UWorld* World = GetWorld() )
	{
		World->GetTimerManager().ClearTimer(EnterEndTimer);
		World->GetTimerManager().ClearTimer(WarningTimer);
		World->GetTimerManager().ClearTimer(GraceEndTimer);
	}

	for ( const EYSBreakPhase Phase : ActivePhases.Array() )
	{
		EndPhase(Phase);
	}
}

void UYSGroggyComponent::OnPhaseChanged(AActor* PhaseOwner, int32 NewPhase)
{
	if ( PhaseOwner != GetOwner() )
	{
		return;
	}

	BreakCount = 0;
	ApplyMaxGroggy();
}

void UYSGroggyComponent::CaptureBaseMaxGroggy()
{
	const UYSGroggyAttributeSet* GroggySet = GetGroggySet();

	if ( BaseMaxGroggy <= 0.f && GroggySet != nullptr )
	{
		BaseMaxGroggy = GroggySet->GetMaxGroggy();
	}
}

void UYSGroggyComponent::ApplyMaxGroggy()
{
	UAbilitySystemComponent* ASC = CachedASC.Get();

	if ( ASC == nullptr || BaseMaxGroggy <= 0.f )
	{
		return;
	}

	const FYSGroggyRules& Rules = GetRules();
	const float Growth = FMath::Min(BreakCount * Rules.MaxGrowthPerBreak, Rules.MaxGrowthCap);
	const float NewMax = BaseMaxGroggy * (1.f + Growth);

	ASC->SetNumericAttributeBase(UYSGroggyAttributeSet::GetMaxGroggyAttribute(), NewMax);

	const float Groggy = ASC->GetNumericAttribute(UYSGroggyAttributeSet::GetGroggyAttribute());
	if ( Groggy > NewMax )
	{
		ASC->SetNumericAttributeBase(UYSGroggyAttributeSet::GetGroggyAttribute(), NewMax);
	}
}

void UYSGroggyComponent::Regenerate(float DeltaTime)
{
	UAbilitySystemComponent* ASC = CachedASC.Get();
	const UYSGroggyAttributeSet* GroggySet = GetGroggySet();
	const UWorld* World = GetWorld();

	if ( ASC == nullptr || GroggySet == nullptr || World == nullptr )
	{
		return;
	}

	const float Groggy = GroggySet->GetGroggy();
	const float MaxGroggy = GroggySet->GetMaxGroggy();

	if ( Groggy >= MaxGroggy )
	{
		return;
	}

	const FYSGroggyRules& Rules = GetRules();

	if ( World->GetTimeSeconds() - GroggySet->GetLastDamageTime() < Rules.RegenDelay )
	{
		return;
	}

	const float NewGroggy = FMath::Min(Groggy + MaxGroggy * Rules.RegenRatePerSecond * DeltaTime, MaxGroggy);
	ASC->SetNumericAttributeBase(UYSGroggyAttributeSet::GetGroggyAttribute(), NewGroggy);
}
