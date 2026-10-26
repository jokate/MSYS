// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/Ability/YSGameplayAbility_ContextSwitch.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "YSDeveloperSettings.h"
#include "Ability/AbilityComponent/YSAbilityPlayback.h"
#include "Ability/AbilityComponent/YSPlaybackTask.h"
#include "Ability/MontageSelector/YSMontageSelector.h"
#include "Character/Components/YSCombatTargetComponent.h"
#include "Combat/YSContextSwitchVariant.h"
#include "Subsystem/YSSquadSubsystem.h"

namespace
{
	// 스위치 창 입력 상태의 이름 (UYSSwitchState::StateName). 상태 해석이 "상태.원본입력" 태그로 어빌리티를 찾는다.
	const FString SwitchStatePrefix = TEXT("Switch.");

	UYSSquadSubsystem* GetSquad(const AActor* Actor)
	{
		const UGameInstance* GameInstance = IsValid(Actor) ? Actor->GetGameInstance() : nullptr;
		return GameInstance ? GameInstance->GetSubsystem<UYSSquadSubsystem>() : nullptr;
	}
}

bool UYSGameplayAbility_ContextSwitch::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if ( Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) == false )
	{
		return false;
	}

	AActor* UnusedTarget = nullptr;
	const FName Row = ResolveRow(Handle, ActorInfo, UnusedTarget);
	const FYSContextSwitchVariant* Variant = GetDefault<UYSDeveloperSettings>()->FindContextSwitchVariant(Row);

	if ( Variant == nullptr )
	{
		return false;
	}

	// 몽타주가 노드를 끝낸다. 없으면 변형이 끝나지 않으므로 발동하지 않고 평소 입력으로 넘긴다.
	if ( Variant->Montage.LoadSynchronous() == nullptr )
	{
		UE_LOG(LogTemp, Warning, TEXT("[ContextSwitch] 변형 행 '%s' 의 몽타주가 비었거나 로드되지 않아 발동하지 않는다."), *Row.ToString());
		return false;
	}

	return true;
}

void UYSGameplayAbility_ContextSwitch::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// 변형 시작 시점에 한 번 판정하고 그 타겟 · 배율을 끝까지 쓴다. 플레이백이 Super 에서 시작하므로 그 전에 정한다.
	AActor* Target = nullptr;
	SelectedRow = ResolveRow(Handle, ActorInfo, Target);
	VariantTarget = Target;

	AActor* Avatar = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr;
	UYSSquadSubsystem* Squad = GetSquad(Avatar);
	EffectScale = Squad ? Squad->GetSwitchEffectScale(Target) : 1.f;

	const FYSContextSwitchVariant* Variant = GetSelectedVariant();

	FYSSwitchAttackReport Report;

	if ( Variant != nullptr )
	{
		const UYSCombatTargetComponent* CombatTarget = UYSCombatTargetComponent::Get(Avatar);

		Report.Trigger = EYSSwitchTrigger::TargetSwitch;
		Report.Row = SelectedRow;
		Report.Impact = Variant->Impact;
		Report.EffectScale = EffectScale;
		Report.bFirstGrab = IsValid(CombatTarget) && CombatTarget->GetManualSwitchSnapshot().bFirstGrab;
		Report.Target = Target;
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 보고는 변형이 실제로 나갔을 때만. 몽타주 재생이 실패하면 체인이 Super 안에서 동기로 끝나 몽타주가 남지 않는다.
	if ( Squad != nullptr && Variant != nullptr && IsActive() && GetCurrentMontage() != nullptr )
	{
		Squad->ReportSwitchAttack(Report);
	}
}

void UYSGameplayAbility_ContextSwitch::BuildDefaultPlaybacks(TArray<TObjectPtr<UYSAbilityPlaybackBase>>& OutPlaybacks)
{
	// 경로를 두지 않는다. 몽타주 완료 · 중단은 Resolve 가 체인을 끝내고, 판정 출력은 전환 없이 지나간다.
	UYSAbilityPlaybackBase* Node = NewObject<UYSAbilityPlaybackBase>(this);

	UYSPlaybackTask_PlayMontage* Montage = NewObject<UYSPlaybackTask_PlayMontage>(Node);
	Montage->MontageSelector.InitializeAs<FYSMontageSelector_ContextSwitch>();

	Node->Tasks.Add(Montage);
	Node->Tasks.Add(NewObject<UYSPlaybackTask_ContextSwitchHit>(Node));

	OutPlaybacks.Add(Node);
}

const FYSContextSwitchVariant* UYSGameplayAbility_ContextSwitch::GetSelectedVariant() const
{
	return GetDefault<UYSDeveloperSettings>()->FindContextSwitchVariant(SelectedRow);
}

FName UYSGameplayAbility_ContextSwitch::ResolveRow(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, AActor*& OutTarget) const
{
	OutTarget = nullptr;

	AActor* Avatar = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr;
	UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	const FGameplayAbilitySpec* Spec = IsValid(ASC) ? ASC->FindAbilitySpecFromHandle(Handle) : nullptr;

	if ( IsValid(Avatar) == false || Spec == nullptr )
	{
		return NAME_None;
	}

	// 태그 교대가 연 창이면 진입기 소관이다.
	const UYSSquadSubsystem* Squad = GetSquad(Avatar);

	if ( Squad == nullptr || Squad->GetLastOpenTrigger() != EYSSwitchTrigger::TargetSwitch )
	{
		return NAME_None;
	}

	// 창을 연 수동 전환의 타겟이 지금 타겟일 때만. 자동 전환은 기록을 남기지 않는다.
	UYSCombatTargetComponent* CombatTarget = UYSCombatTargetComponent::Get(Avatar);
	AActor* Target = IsValid(CombatTarget) ? CombatTarget->GetCurrentTarget() : nullptr;

	if ( IsValid(Target) == false || CombatTarget->GetManualSwitchSnapshot().Target.Get() != Target )
	{
		return NAME_None;
	}

	const UYSDeveloperSettings* Settings = GetDefault<UYSDeveloperSettings>();

	// 창 상한보다 오래된 기록은 지금 창을 연 전환이 아니다.
	const float WindowDuration = Settings->GetSwitchWindowDuration();

	if ( WindowDuration > 0.f && Avatar->GetWorld()->GetTimeSeconds() - CombatTarget->GetManualSwitchSnapshot().Time > WindowDuration )
	{
		return NAME_None;
	}

	const FYSContextSwitchJudgeRules& Rules = Settings->ContextSwitchJudgeRules;
	const FVector ToTarget = Target->GetActorLocation() - Avatar->GetActorLocation();

	FYSContextSwitchSituation Situation;

	if ( const UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target) )
	{
		TargetASC->GetOwnedGameplayTags(Situation.TargetTags);
	}

	Situation.Height = Rules.ResolveHeight(ToTarget.Z);
	Situation.Distance = CombatTarget->GetDistanceBand(Target);
	Situation.Direction = Rules.ResolveDirection(CombatTarget->GetManualSwitchSnapshot().PreSwitchForward, ToTarget);

	const FGameplayTagContainer& SpecInputs = Spec->GetDynamicSpecSourceTags();

	const FName Row = FYSContextSwitchVariant::FindBestRow(Settings->ContextSwitchVariantTable.LoadSynchronous(),
		[&SpecInputs](const FYSContextSwitchVariant& Variant)
		{
			for ( const FGameplayTag& Input : Variant.TriggerInputs )
			{
				const FGameplayTag StateInput = FGameplayTag::RequestGameplayTag(FName(SwitchStatePrefix + Input.ToString()), false);

				if ( StateInput.IsValid() && SpecInputs.HasTagExact(StateInput) )
				{
					return true;
				}
			}
			return false;
		}, Situation);

	if ( Row.IsNone() == false )
	{
		OutTarget = Target;
	}

	return Row;
}
