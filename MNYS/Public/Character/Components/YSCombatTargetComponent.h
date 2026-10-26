// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "General/YSEnum.h"
#include "YSCombatTargetComponent.generated.h"

class UYSCombatProfile;
class UYSInputStateMachineComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FYSOnCombatTargetChanged, AActor*, OldTarget, AActor*, NewTarget, EYSTargetChangeReason, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FYSOnNearbyEnemiesChanged, const TArray<AActor*>&, NearbyEnemies);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FYSOnEngagementChanged);

/**
 * Q/E 수동 전환(첫 잡기 포함) 한 번의 기록. 컨텍스트 스위치 변형이 방향을 잴 때 쓴다.
 * 캐릭터는 타겟이 바뀐 프레임에 바로 돌기 때문에 정면은 SetTarget 전에 남긴다.
 */
struct FYSManualSwitchSnapshot
{
	TWeakObjectPtr<AActor> Target;
	FVector PreSwitchForward = FVector::ForwardVector;
	float Time = -BIG_NUMBER;
	// 전환 전에 타겟이 없었다.
	bool bFirstGrab = false;
};

/**
 * 플레이어 공격의 대상을 고른다 (기획 02_COMBAT_TARGETING_DRAFT §3~§8).
 *
 * 컨트롤러에 둔다. 스쿼드 태그 교대로 폰이 바뀌어도 타겟이 이어져야 하기 때문이다.
 * 선택만 한다 — 카메라(락온)는 OnTargetChanged 를 구독해 따라가기만 한다.
 * 주변 적 감지(반경 · 가림)로 교전 시작 · 종료도 판정한다. 교전이 시작될 때 타겟이 없으면
 * 같은 점수 규칙으로 하나 고르고, 교전이 끝나면 비운다. HUD 는 감지 목록을 구독해 레이더에 내린다.
 * 캐릭터마다 다른 수치는 UYSCombatProfile 에만 있다.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MNYS_API UYSCombatTargetComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UYSCombatTargetComponent();

	/** 폰이나 컨트롤러 어느 쪽을 넘겨도 찾는다. */
	static UYSCombatTargetComponent* Get(const AActor* PawnOrController);

	UFUNCTION(BlueprintPure, Category = "YS | CombatTarget")
	AActor* GetCurrentTarget() const { return CurrentTarget.Get(); }

	/**
	 * 공격 시점 평가(§3). 점수가 가장 높은 후보로 바꾸고 그 대상을 돌려준다.
	 * 저스트 회피 창 중에는 평가를 건너뛰고 지금 타겟을 그대로 낸다(§4).
	 */
	UFUNCTION(BlueprintCallable, Category = "YS | CombatTarget")
	AActor* RefreshTarget();

	/**
	 * 수동 전환(§6). 후보를 화면 왼쪽→오른쪽으로 세워 다음/이전으로 넘기고 끝에서 처음으로 돈다.
	 * 지금 타겟 말고 고를 후보가 없으면 아무것도 하지 않는다.
	 * @return 타겟이 바뀌었으면 true
	 */
	UFUNCTION(BlueprintCallable, Category = "YS | CombatTarget")
	bool CycleTarget(bool bNext);

	UFUNCTION(BlueprintCallable, Category = "YS | CombatTarget")
	void ClearTarget();

	/** 대상까지의 거리 구간. 지금 타겟이면 이력(떨림 방지)을 적용한다. */
	EYSTargetDistanceBand GetDistanceBand(const AActor* Target);

	const UYSCombatProfile* GetActiveProfile() const { return ActiveProfile.Get(); }

	const FYSManualSwitchSnapshot& GetManualSwitchSnapshot() const { return ManualSwitchSnapshot; }

	UFUNCTION(BlueprintPure, Category = "YS | CombatTarget")
	bool IsEngaged() const { return bEngaged; }

	UFUNCTION(BlueprintPure, Category = "YS | CombatTarget")
	TArray<AActor*> GetNearbyEnemies() const;

	UPROPERTY(BlueprintAssignable, Category = "YS | CombatTarget")
	FYSOnCombatTargetChanged OnTargetChanged;

	// 감지된 적 목록이 바뀔 때. 교전 시작보다 먼저 온다.
	UPROPERTY(BlueprintAssignable, Category = "YS | CombatTarget")
	FYSOnNearbyEnemiesChanged OnNearbyEnemiesChanged;

	// 감지된 적이 0명에서 1명 이상이 될 때.
	UPROPERTY(BlueprintAssignable, Category = "YS | CombatTarget")
	FYSOnEngagementChanged OnEngagementBegan;

	// 감지된 적 0명이 교전 종료 대기 시간만큼 이어질 때.
	UPROPERTY(BlueprintAssignable, Category = "YS | CombatTarget")
	FYSOnEngagementChanged OnEngagementEnded;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UFUNCTION()
	void HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);

	UFUNCTION()
	void HandleRawInput(const FGameplayTag& InputTag, EYSInputPhase InputPhase);

	void BindPawn(APawn* Pawn);
	void UnbindPawn();
	void ResolveProfile();

	APawn* GetOwnerPawn() const;
	bool GetViewPoint(FVector& OutLocation, FRotator& OutRotation) const;

	void GatherCandidates(TArray<AActor*>& OutCandidates) const;

	/** 사정거리 · 시야각 · 가림 · 제외 상태(§4). 모두 통과해야 후보다. */
	bool IsValidCandidate(const AActor* Candidate) const;
	bool IsExcluded(const AActor* Candidate) const;

	float ScoreCandidate(const AActor* Candidate) const;
	AActor* SelectBest(const TArray<AActor*>& Candidates) const;

	void SetTarget(AActor* NewTarget, EYSTargetChangeReason Reason);

	/** 자동 전환(§6). 타겟이 죽거나 조건을 벗어나면 유지 시간을 본 뒤 차선으로 넘긴다. */
	void MaintainTarget();

	bool IsJustAvoidWindowActive() const;

	/** 주변 적 감지. 반경 = 공격 사정거리, 각도는 감지 시야각(360 이면 끔), 가림 · 제외는 후보와 같은 기준이다. */
	void ScanNearbyEnemies();
	bool IsAware(const AActor* Candidate) const;
	bool HasSameNearbyEnemies(const TArray<AActor*>& Found) const;
	void UpdateEngagement(bool bAnyEnemy);

	void BeginEngagement();
	void EndEngagement();

	// 교전 중 타겟을 잃으면 다시 잡기를 시작한다.
	UFUNCTION()
	void HandleOwnTargetChanged(AActor* OldTarget, AActor* NewTarget, EYSTargetChangeReason Reason);

	/**
	 * 교전 중 타겟이 없으면 점수 규칙으로 하나 고른다. 잡았거나 교전이 아니면 재시도를 멈춘다.
	 * 등 뒤 적으로 교전이 시작되면 시야각 밖이라 후보가 없어, 돌아볼 때까지 유지 검사 간격으로 다시 고른다.
	 */
	void TryEngageTarget();
	void StartEngageRetry();
	void StopEngageRetry();

private:
	TWeakObjectPtr<AActor> CurrentTarget;

	TWeakObjectPtr<UYSInputStateMachineComponent> BoundInputStateMachine;

	TWeakObjectPtr<const UYSCombatProfile> ActiveProfile;

	// 설정의 기본 프로필은 소프트 참조로 로드해 오므로 쥐고 있지 않으면 GC 로 사라질 수 있다.
	UPROPERTY(Transient)
	TObjectPtr<UYSCombatProfile> LoadedDefaultProfile;

	// 지금 타겟이 조건을 벗어난 시각. 음수면 조건 안이다.
	float LostSinceTime = -1.f;

	// 파괴로 약참조가 끊긴 것을 "원래 없던 것" 과 구분하려고 둔다.
	bool bHasTarget = false;

	EYSTargetDistanceBand LastBand = EYSTargetDistanceBand::OutOfRange;
	bool bHasLastBand = false;

	bool bWarnedMissingProfile = false;

	FYSManualSwitchSnapshot ManualSwitchSnapshot;

	TArray<TWeakObjectPtr<AActor>> NearbyEnemies;

	bool bEngaged = false;

	// 감지된 적이 0명이 된 시각. 음수면 누군가 감지돼 있다.
	float EmptySinceTime = -1.f;

	FTimerHandle EngageRetryTimer;

	UPROPERTY(EditDefaultsOnly, Category = "YS | CombatTarget", meta = (DisplayName = "자동 유지 검사 간격 (초)", ClampMin = "0"))
	float MaintainInterval = 0.1f;
};
