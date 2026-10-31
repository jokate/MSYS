// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/Ability/YSGameplayAbility_ContextSwitch.h"

#include "Animation/AnimMontage.h"
#include "YSDeveloperSettings.h"
#include "Character/Components/YSCombatTargetComponent.h"
#include "Subsystem/YSSquadSubsystem.h"

namespace
{
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

	AActor* Target = FindSwitchTarget(ActorInfo);

	if ( IsValid(Target) == false )
	{
		return false;
	}

	// 맞는 경로가 없으면 발동하지 않고 평소 입력으로 넘긴다. 발동 뒤에 알면 입력만 먹고 아무것도 안 나간다.
	const TSharedPtr<FYSPlaybackContext> Context = MakeShared<FYSPlaybackContext>();
	Context->Instigator = ActorInfo->AvatarActor.Get();
	Context->Target = Target;

	if ( CanStartPlayback(Context) == false )
	{
		UE_LOG(LogTemp, Log, TEXT("[ContextSwitch] %s: 첫 노드에 맞는 경로가 없어 발동하지 않는다 (타겟 %s)."), *GetName(), *Target->GetName());
		return false;
	}

	return true;
}

void UYSGameplayAbility_ContextSwitch::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// 발동 시점에 한 번 정하고 그 타겟 · 배율을 끝까지 쓴다. 플레이백이 Super 에서 시작하므로 그 전에 정한다.
	AActor* Target = FindSwitchTarget(ActorInfo);
	SwitchTarget = Target;

	AActor* Avatar = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr;
	UYSSquadSubsystem* Squad = GetSquad(Avatar);
	EffectScale = Squad ? Squad->GetSwitchEffectScale(Target) : 1.f;

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 보고는 모션이 실제로 나갔을 때만. 몽타주 재생이 실패하면 체인이 Super 안에서 동기로 끝나 몽타주가 남지 않는다.
	const UAnimMontage* Montage = GetCurrentMontage();

	UE_LOG(LogTemp, Log, TEXT("[ContextSwitch] %s: 발동 — 노드 %d, 몽타주 %s"), *GetName(), GetCurrentPlaybackIndex(), Montage ? *Montage->GetName() : TEXT("없음"));

	if ( Squad == nullptr || IsValid(Target) == false || IsActive() == false || Montage == nullptr )
	{
		return;
	}

	const UYSCombatTargetComponent* CombatTarget = UYSCombatTargetComponent::Get(Avatar);

	FYSSwitchAttackReport Report;
	Report.Trigger = EYSSwitchTrigger::TargetSwitch;
	Report.Row = Montage->GetFName();
	// 등급은 몽타주 노티파이의 피해 행이 갖는다. 발동 때는 알 수 없다.
	Report.Impact = EYSImpactGrade::None;
	Report.EffectScale = EffectScale;
	Report.bFirstGrab = IsValid(CombatTarget) && CombatTarget->GetManualSwitchSnapshot().bFirstGrab;
	Report.Target = Target;

	Squad->ReportSwitchAttack(Report);
}

void UYSGameplayAbility_ContextSwitch::SetupPlayBack(const FGameplayEventData* TriggerEventData)
{
	// 첫 노드의 분기 조건 · 워프가 이 타겟을 읽는다. 재생이 Super 안에서 시작되므로 그 전에 넣는다.
	PlaybackContext->Target = SwitchTarget.Get();

	Super::SetupPlayBack(TriggerEventData);
}

AActor* UYSGameplayAbility_ContextSwitch::FindSwitchTarget(const FGameplayAbilityActorInfo* ActorInfo) const
{
	AActor* Avatar = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr;

	if ( IsValid(Avatar) == false )
	{
		return nullptr;
	}

	// 태그 교대가 연 창이면 진입기 소관이다.
	const UYSSquadSubsystem* Squad = GetSquad(Avatar);

	if ( Squad == nullptr || Squad->GetLastOpenTrigger() != EYSSwitchTrigger::TargetSwitch )
	{
		UE_LOG(LogTemp, Log, TEXT("[ContextSwitch] 마지막 창이 Q/E 수동 전환이 연 창이 아니다."));
		return nullptr;
	}

	// 창을 연 수동 전환의 타겟이 지금 타겟일 때만. 자동 전환은 기록을 남기지 않는다.
	UYSCombatTargetComponent* CombatTarget = UYSCombatTargetComponent::Get(Avatar);
	AActor* Target = IsValid(CombatTarget) ? CombatTarget->GetCurrentTarget() : nullptr;

	if ( IsValid(Target) == false || CombatTarget->GetManualSwitchSnapshot().Target.Get() != Target )
	{
		UE_LOG(LogTemp, Log, TEXT("[ContextSwitch] 지금 타겟이 수동 전환 기록의 타겟과 다르다."));
		return nullptr;
	}

	// 창 상한보다 오래된 기록은 지금 창을 연 전환이 아니다.
	const float WindowDuration = GetDefault<UYSDeveloperSettings>()->GetSwitchWindowDuration();
	const float Elapsed = Avatar->GetWorld()->GetTimeSeconds() - CombatTarget->GetManualSwitchSnapshot().Time;

	if ( WindowDuration > 0.f && Elapsed > WindowDuration )
	{
		UE_LOG(LogTemp, Log, TEXT("[ContextSwitch] 수동 전환 뒤 %.2f초 — 창 상한 %.2f초를 넘었다."), Elapsed, WindowDuration);
		return nullptr;
	}

	return Target;
}
