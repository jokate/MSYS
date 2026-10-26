// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "Combat/YSSwitchAbusePolicy.h"
#include "Data/YSDataStruct.h"
#include "General/YSEnum.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "YSSquadSubsystem.generated.h"

/**
 * 
 */

class UAbilitySystemComponent;
class UYSAbilitySystemComponent;
struct FYSCharacterInfo;
/** 스쿼드 1인분의 런타임 상태 */
USTRUCT()
struct FYSSquadSlot
{
	GENERATED_BODY()

	UPROPERTY()
	FYSCharacterInfo CharacterInfo;
	
	UPROPERTY()
	TObjectPtr<APawn> Pawn = nullptr;
	
	FActiveGameplayEffectHandle BackupHandleOnField;
};

/** 조기 닫기 조건을 보고 있는 창 하나. 조건이 있는 트리거로 연 창만 남긴다. */
struct FYSTrackedSwitchWindow
{
	FActiveGameplayEffectHandle Handle;
	TWeakObjectPtr<UAbilitySystemComponent> ASC;
	TWeakObjectPtr<AActor> Target;
	EYSSwitchTrigger Trigger = EYSSwitchTrigger::TargetSwitch;
	float OpenTime = 0.f;
};

/**
 * 스위치 창 안에서 나간 공격 하나. 공격 쪽이 발동 때 채워 ReportSwitchAttack 으로 낸다.
 * 지금은 컨텍스트 스위치 변형만 낸다. 다른 스위치 공격(교대 진입기 등)도 같은 보고를 채운다.
 */
USTRUCT(BlueprintType)
struct FYSSwitchAttackReport
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "YS | Squad")
	EYSSwitchTrigger Trigger = EYSSwitchTrigger::TargetSwitch;

	// 표의 행 이름 (변형 표 · 진입기 형 표).
	UPROPERTY(BlueprintReadOnly, Category = "YS | Squad")
	FName Row = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "YS | Squad")
	EYSImpactGrade Impact = EYSImpactGrade::M;

	// 남용 감쇠 배율. 1 이면 감쇠 없음.
	UPROPERTY(BlueprintReadOnly, Category = "YS | Squad")
	float EffectScale = 1.f;

	// 창을 연 전환이 타겟이 없던 상태의 첫 잡기였다.
	UPROPERTY(BlueprintReadOnly, Category = "YS | Squad")
	bool bFirstGrab = false;

	UPROPERTY(BlueprintReadOnly, Category = "YS | Squad")
	TObjectPtr<AActor> Target = nullptr;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FYSOnTagGaugeChanged, float, NewGauge);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FYSOnActiveSlotChanged, int32, OldSlot, int32, NewSlot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FYSOnSwitchWindowOpened, EYSSwitchTrigger, Trigger, AActor*, Target, float, AbuseCooldown);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FYSOnSwitchAttackFired, const FYSSwitchAttackReport&, Report);


UCLASS()
class MNYS_API UYSSquadSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
	
public :
	UYSSquadSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** 처음에 스쿼드 구성한다.
	 * 런 시작 시 3인 스쿼드를 구성한다.
	 * @param Definitions  선택된 캐릭터 3인. 0번이 시작 캐릭터.
	 * @param SpawnTransform  최초 스폰 위치
	 */
	UFUNCTION(BlueprintCallable, Category = "YS | Squad")
	void InitializeSquad(const TArray<FYSCharacterInfo>& Definitions, const FTransform& SpawnTransform);
	
	/**
	 * 태그에 대한 요청을 처리 
	 */
	UFUNCTION(BlueprintCallable, Category = "YS | Squad")
	bool RequestTag(int32 SlotIndex, bool bSwitchAttack);

	/** 현재 캐릭터가 다운됐을 때 호출. 쿨다운을 무시하고 살아있는 슬롯으로 강제 교대한다. */
	UFUNCTION(BlueprintCallable, Category = "YS | Squad")
	bool ForceTagOnDown();

	UFUNCTION(BlueprintPure, Category = "YS | Squad")
	bool IsTagOnCooldown() const;

	UFUNCTION(BlueprintPure, Category = "YS | Squad")
	APawn* GetActivePawn() const;

	UFUNCTION(BlueprintPure, Category = "YS | Squad")
	int32 GetActiveSlot() const { return ActiveSlot; }

	UFUNCTION(BlueprintPure, Category = "YS | Squad")
	int32 GetSlotCount() const { return Slots.Num(); }

	/**
	 * 슬롯의 정의와 ASC를 함께 꺼낸다. 해금 시스템이 캐릭터를 찾을 때 쓴다.
	 * @return 슬롯이 유효하면 true
	 */
	bool GetSlotInfo(int32 SlotIndex, FYSCharacterInfo& OutDefinition, UYSAbilitySystemComponent*& OutASC) const;

	/** 현재 필드 캐릭터의 ASC. 스쿼드가 비었으면 조종 중인 폰의 ASC 다. */
	UFUNCTION(BlueprintPure, Category = "YS | Squad")
	UYSAbilitySystemComponent* GetActiveASC() const;
	
	UFUNCTION(BlueprintCallable, Category = "YS | Squad")
	void AddTagGauge(float Amount);

	/** 게이지를 소모한다. 부족하면 false를 반환하고 아무것도 소모하지 않는다. */
	UFUNCTION(BlueprintCallable, Category = "YS | Squad")
	bool TryConsumeGauge(float Cost);

	UFUNCTION(BlueprintPure, Category = "YS | Squad")
	float GetTagGauge() const { return TagGauge; }

	/**
	 * 스위치 창을 연다. 어느 트리거가 여는지 · 남용 제어는 YS Developer Settings 를 따른다.
	 * @param BaseCost  정책을 거치기 전 게이지 소모량
	 * @return 창이 열렸으면 true
	 */
	UFUNCTION(BlueprintCallable, Category = "YS | Squad")
	bool TryOpenSwitchWindow(EYSSwitchTrigger Trigger, AActor* Target, float BaseCost);

	/** 스위치 어택의 경직 · 그로기 감소에 곱할 배율 (남용 정책 ② 가 쓴다). 적중 쪽에서 읽는다. */
	UFUNCTION(BlueprintPure, Category = "YS | Squad")
	float GetSwitchEffectScale(const AActor* Target) const;

	/** 이 대상에게 남용 제어가 걸려 있는 남은 시간. 대상을 따지지 않는 정책이면 대상과 상관없다. HUD 가 타겟을 바꿀 때 읽는다. */
	float GetSwitchAbuseCooldownRemaining(const AActor* Target) const;

	/** 마지막으로 스위치 창을 연 트리거. 창 안 입력이 어느 쪽 공격인지 가를 때 읽는다. */
	EYSSwitchTrigger GetLastOpenTrigger() const { return LastOpenTrigger; }

	/** 열려 있는 스위치 창 GE 를 걷는다. 다운 때 부른다. */
	void CloseSwitchWindow(UAbilitySystemComponent* ASC) const;

	/** 스위치 창 안 공격이 나갔다고 알린다. 적중 전, 발동 시점에 부른다. */
	void ReportSwitchAttack(const FYSSwitchAttackReport& Report);

	UPROPERTY(BlueprintAssignable, Category = "YS | Squad")
	FYSOnTagGaugeChanged OnTagGaugeChanged;

	UPROPERTY(BlueprintAssignable, Category = "YS | Squad")
	FYSOnActiveSlotChanged OnActiveSlotChanged;

	/** 창 GE 를 건 직후. AbuseCooldown 은 남용 정책상 다시 열 수 있을 때까지 남은 시간이다. */
	UPROPERTY(BlueprintAssignable, Category = "YS | Squad")
	FYSOnSwitchWindowOpened OnSwitchWindowOpened;

	UPROPERTY(BlueprintAssignable, Category = "YS | Squad")
	FYSOnSwitchAttackFired OnSwitchAttackFired;

protected:
	void PerformTag(int32 NewSlot, bool bSwitchAttack, bool bOpenSwitchWindow = false);

	/** 창을 열 수 있는지 보고 이번 소모량을 낸다. 상태는 바꾸지 않는다. */
	bool EvaluateSwitchWindow(EYSSwitchTrigger Trigger, const AActor* Target, float BaseCost, float& OutCost) const;

	/** 창 GE 를 걸고 개시 기록을 남긴다. 게이지는 호출한 쪽이 이미 썼다. GE 가 안 걸렸으면 경고를 남기고 false. */
	bool OpenSwitchWindow(EYSSwitchTrigger Trigger, UYSAbilitySystemComponent* ASC, AActor* Target);

	/** 이 트리거가 읽고 쓸 남용 기록. 설정의 기록 범위가 공유면 모든 트리거가 한 벌을 쓴다. */
	const FYSSwitchAbuseState& GetAbuseState(EYSSwitchTrigger Trigger) const;
	EYSSwitchTrigger GetAbuseStateKey(EYSSwitchTrigger Trigger) const;
	void ResetSwitchAbuseStates();

	/** 조건이 있는 트리거면 창을 기록하고 다음 틱부터 본다. 같은 트리거의 이전 창은 새 창이 대신한다. */
	void TrackSwitchWindow(EYSSwitchTrigger Trigger, UAbilitySystemComponent* ASC, FActiveGameplayEffectHandle Handle, AActor* Target);

	/** 열린 동안 매 틱 조기 닫기 조건을 본다. 창이 먼저 닫혔으면(상한 · 다운 · 입력) 기록만 지운다. */
	void PollTrackedSwitchWindows();

	void HandleWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources);

	AActor* GetCurrentCombatTarget() const;
	
	void RefreshBackupPassives();
	
	void SetPawnActive(APawn* Pawn, bool bActive);

	UYSAbilitySystemComponent* GetASC(int32 SlotIndex) const;;

private:
	UPROPERTY()
	TArray<FYSSquadSlot> Slots;

	UPROPERTY()
	int32 ActiveSlot = 0;

	UPROPERTY()
	float TagGauge = 0.f;

	FTimerHandle TagCooldownTimer;

	TMap<EYSSwitchTrigger, FYSSwitchAbuseState> SwitchAbuseStates;

	// 효과 배율(GetSwitchEffectScale)은 지금 열린 창을 연 트리거의 기록으로 읽는다.
	EYSSwitchTrigger LastOpenTrigger = EYSSwitchTrigger::TagSwap;

	TArray<FYSTrackedSwitchWindow> TrackedSwitchWindows;

	FTimerHandle SwitchWindowPollTimer;

	FDelegateHandle WorldCleanupHandle;
};