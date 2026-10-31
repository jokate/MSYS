// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "Components/ActorComponent.h"
#include "General/YSEnum.h"
#include "YSGroggyComponent.generated.h"

class UAbilitySystemComponent;
class UAnimMontage;
class UYSGroggyAttributeSet;
class UYSGroggyProfile;
struct FGameplayEffectRemovalInfo;
struct FGameplayEventData;
struct FYSGroggyRules;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FYSOnBreakPhaseChanged, EYSBreakPhase, Phase, bool, bActive);

/** 브레이크 규칙을 한 곳에서 갖는다. Event.Groggy.Broken → 어빌리티 취소 · UYSGE_GroggyBreak · 몽타주, 종료 → 최대치 증가(상한) · 게이지 가득 · 유예, 평소엔 회복 지연 뒤 초당 회복, 페이즈 전환 때 최대치 초기화. 연출은 단계의 시작과 끝만 정하고 단계마다 Rules.PhaseCues 의 Cue 를 켜고 끈다. 수치는 FYSGroggyRules. */
UCLASS(ClassGroup=(Custom))
class MNYS_API UYSGroggyComponent : public UActorComponent
{
	GENERATED_BODY()

public :
	UYSGroggyComponent();

	void SetProfile(const UYSGroggyProfile* InProfile) { Profile = InProfile; }

	bool IsBroken() const { return BreakHandle.IsValid(); }

	const FYSGroggyRules& GetRules() const;

	// HUD 그로기 바(타이머 · 점멸 · 유예 빗금)가 받는다.
	UPROPERTY(BlueprintAssignable, Category = "YS | Groggy")
	FYSOnBreakPhaseChanged OnBreakPhaseChanged;

protected :
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public :
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private :
	const UYSGroggyAttributeSet* GetGroggySet() const;

	void OnGroggyBroken(const FGameplayEventData* Payload);
	void OnBreakEffectRemoved(const FGameplayEffectRemovalInfo& RemovalInfo);
	void OnPhaseChanged(AActor* PhaseOwner, int32 NewPhase);

	void CancelActiveAbilities(const FYSGroggyRules& Rules);
	void ApplyBreakEffect(const FYSGroggyRules& Rules);
	void PlayBreakMontage(const FYSGroggyRules& Rules);
	void StopBreakMontage();

	void CaptureBaseMaxGroggy();
	void ApplyMaxGroggy();
	void Regenerate(float DeltaTime);

	void BeginPhase(EYSBreakPhase Phase);
	void EndPhase(EYSBreakPhase Phase);
	void SchedulePhase(FTimerHandle& Handle, EYSBreakPhase Phase, bool bBegin, float Delay);
	void ClearPhases();

private :
	UPROPERTY()
	TObjectPtr<const UYSGroggyProfile> Profile;

	TWeakObjectPtr<UAbilitySystemComponent> CachedASC;
	TWeakObjectPtr<UAnimMontage> PlayingMontage;

	FActiveGameplayEffectHandle BreakHandle;
	FDelegateHandle BrokenEventHandle;
	FDelegateHandle PhaseChangedHandle;

	// Cue 의 Instigator. 카메라 · 흔들림 트랙이 이 쪽 화면을 고른다.
	TWeakObjectPtr<AActor> BreakInstigator;
	TSet<EYSBreakPhase> ActivePhases;
	FTimerHandle EnterEndTimer;
	FTimerHandle WarningTimer;
	FTimerHandle GraceEndTimer;

	// 초기화 GE 가 BeginPlay 뒤에 최대치를 덮을 수 있어, 첫 브레이크 때 잡는다.
	float BaseMaxGroggy = 0.f;
	int32 BreakCount = 0;
};
