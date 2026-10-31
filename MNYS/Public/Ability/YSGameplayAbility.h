// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Data/YSDataStruct.h"
#include "General/YSEnum.h"
#include "General/YSMacros.h"
#include "General/YSStruct.h"
#include "Input/Combo/YSComboData.h"
#include "Interface/YSLinkedActor.h"
#include "YSGameplayAbility.generated.h"

struct FYSPlaybackContext;
class UYSGameplayAbility;
class UYSAbilityPlaybackBase;
class UYSPlaybackGraphAsset;
class UYSAT_Trace;
struct FYSMontageSelector;
class UYSAbilityEventAction;
struct FYSComboTransition;

struct FYSAbilityHitContext
{
	FYSAbilityHitContext() {};
	FYSAbilityHitContext(AActor* Instigator, UYSGameplayAbility* InOwningAbility);

	void AddHitActor(AActor* HitActor) { HitActors.Add(HitActor); }
	void UpdateHitResult(UObject* HitObject, const FHitResult& HitResult)
	{
		for (auto It = RecentHitResults.CreateIterator(); It; ++It)
		{
			if (!It.Key().IsValid()) { It.RemoveCurrent(); }
		}

		RecentHitResults.FindOrAdd(HitObject).Add(HitResult);
	}

	/** 기록하고 낸 어빌리티에 '맞혔다'를 보낸다. 어빌리티가 끝난 뒤의 맞힘은 버린다. */
	void ReportHit(UObject* HitObject, const TArray<FHitResult>& HitResults);

	/** 어빌리티가 끝날 때 부른다. 기록을 지우고, 남은 공격 액터 · 취소로 설정된 대기 스폰에 알린다. */
	void OnOwnerEnded();

	bool IsOwnerActive() const { return OwningAbility.IsValid(); }

	/** 어빌리티가 끝난 뒤에 나온 공격 액터도 끝날 때의 배율을 쓴다. */
	float GetEffectScale() const;

	const TArray<FHitResult>* GetHitResultsForObject(UObject* HitObject) const
	{
		if (const TArray<FHitResult>* FoundResults = RecentHitResults.Find(HitObject))
		{
			return FoundResults;
		}
		
		return nullptr;
	}
	
	TArray<AActor*> GetAllHitActors() const
	{
		TArray<AActor*> ValidHitActors;
		for (const TWeakObjectPtr<AActor>& WeakActor : HitActors)
		{
			if (AActor* Actor = WeakActor.Get())
			{
				ValidHitActors.Add(Actor);
			}
		}
		return ValidHitActors;
	}
	void RegisterLinkedActor(AActor* InActor)
	{
		check(IsInGameThread());
		LinkedActors.AddUnique(InActor);
	}
	
	void NotifyLinkedActorEnded(AActor* InActor)
	{
		check(IsInGameThread());
		if (LinkedActors.Remove(InActor) > 0)
		{
			OnLinkedActorEnded.Broadcast(InActor);
		}
	}

	// Release 가 풀 반환을 거쳐 NotifyLinkedActorEnded 로 되돌아와 배열을 건드리므로 사본으로 돈다.
	void ReleaseAllLinkedActors(EYSReleaseReason Reason)
	{
		check(IsInGameThread());
		const TArray<TWeakObjectPtr<AActor>> Snapshot = LinkedActors;
		for (const TWeakObjectPtr<AActor>& WeakActor : Snapshot)
		{
			if (IYSLinkedActor* LinkedActor = Cast<IYSLinkedActor>(WeakActor.Get()))
			{
				LinkedActor->Release(Reason);
			}
		}
	}

	TMulticastDelegate<void(AActor*)> OnLinkedActorEnded;
	TMulticastDelegate<void()> OnOwnerEndedEvent;

private :
	TWeakObjectPtr<AActor> InstigatorActor;
	TWeakObjectPtr<UYSGameplayAbility> OwningAbility;
	float EndedEffectScale = 1.f;
	TSet<TWeakObjectPtr<AActor>> HitActors;
	TMap<TWeakObjectPtr<UObject>, TArray<FHitResult>> RecentHitResults;
	TArray<TWeakObjectPtr<AActor>> LinkedActors;
};

USTRUCT()
struct FYSGameplayAbility_RuntimeData
{
	GENERATED_BODY()

public:
	YS_GETTER_REF(TArray<FActiveGameplayEffectHandle>, ActiveGameplayEffectHandle)
	
	void AddEffectSpecHandle(const FActiveGameplayEffectHandle& Handle)
	{
		ActiveGameplayEffectHandle.Add(Handle);
	}
	
	void RemoveEffectSpecHandle(const FActiveGameplayEffectHandle& Handle)
	{
		ActiveGameplayEffectHandle.Remove(Handle);
	}
	
	void ResetData()
	{
		ActiveGameplayEffectHandle.Empty();
	}

private:
	
	UPROPERTY()
	TArray<FActiveGameplayEffectHandle> ActiveGameplayEffectHandle;
};

USTRUCT(BlueprintType)
struct FYSEventPayload
{
	GENERATED_BODY()

	YS_ACCESSOR(const TArray<UYSAbilityEventAction*>&, EventActions);
	
public :
	UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite, meta = (DisplayName = "이벤트 액션"))
	TArray<TObjectPtr<UYSAbilityEventAction>> EventActions;
};

UCLASS()
class MNYS_API UYSGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UYSGameplayAbility();
	
	UFUNCTION()
	void OnGameplayTagChanged(const FGameplayTag& Tag, bool bInIsActive);

	virtual bool TryTransition(const FGameplayTag& InputGameplayTag, EYSInputPhase InputPhase);
	
	YS_ACCESSOR(UYSAT_Trace*, TraceTask)
	
	void NotifyPlaybackChainFinished();
	UYSAbilityPlaybackBase* GetPlaybackNode(int32 Index);

	/** 그래프 에셋의 사본, 그래프가 없으면 클래스 기본. 첫 호출에서 사본을 만들므로 const 가 아니다 (EnsureRuntimePlaybacks). */
	const TArray<TObjectPtr<UYSAbilityPlaybackBase>>& GetPlaybackArray();

#if WITH_EDITOR
	/** 그래프로 옮기기 전의 레거시 배열. 이전 도구만 쓴다. */
	const TArray<TObjectPtr<UYSAbilityPlaybackBase>>& GetLegacyPlaybacks() const { return Playbacks; }
#endif
	
	const FGameplayEventData* GetEventData() const { return &CurrentEventData; }
	
	/** 어빌리티가 건 GE 를 어빌리티가 끝날 때 제거한다. */
	void AddRuntimeEffectSpecHandle(const FActiveGameplayEffectHandle& Handle);

	/** 어빌리티가 끝날 때 (취소 · 정상 모두) 등록의 역순으로 한 번씩 실행하고 비운다. 람다는 UObject 를 weak 로 잡을 것 — 그 사이 사라질 수 있다. */
	void AddEndCleanup(TFunction<void()> Cleanup) { EndCleanups.Add(MoveTemp(Cleanup)); }

	void ActivePlayback(int32 Index);
	
	UYSAbilityPlaybackBase* GetCurrentPlayback() const { return CurrentPlayback.Get(); }
	int32 GetCurrentPlaybackIndex() const { return CurrentPlaybackIndex; }
	
	YS_BOOL_ACCESSOR(bIsInputAcceptable, InputAcceptable)
	
	const TSharedPtr<FYSAbilityHitContext>& GetHitContext() const { return HitContext; }
	const TSharedPtr<FYSPlaybackContext>& GetAbilityPlaybackContext() const { return PlaybackContext; }

	/** 이 어빌리티의 피해 · 그로기 피해에 곱하는 배율. 확정 히트 · 트레이스가 피해 함수에 넘긴다. */
	virtual float GetEffectScale() const { return 1.f; }

	virtual bool CheckCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	UFUNCTION()
	virtual void OnMontagePlayed();

	UFUNCTION()
	void OnMontageInterrupted();
	
	virtual void SetupPlayBack(const FGameplayEventData* TriggerEventData);

	/** 그래프 첫 노드가 이 컨텍스트로 시작할 수 있는가. 그래프가 없으면 거짓. */
	bool CanStartPlayback(const TSharedPtr<FYSPlaybackContext>& Context) const;

	const FYSSkillInfo* GetSkillInfo() const;

	bool HasCommitOnEnterNode();

private :
	UFUNCTION()
	void _ProcessEvent(FGameplayEventData Payload);
	
	void _PrepareForAbilityEvent();
	
	void _AddWorldTag();
	void _RemoveWorldTag();


public:
	// 어빌리티가 켜져 있는 동안의 입력 상태 (입력 상태 정의 표 행). 비우면 바꾸지 않는다. 입력 상태 머신이 있는 주인만.
	UPROPERTY(EditDefaultsOnly, Category = "YS | Input Related", meta = (DisplayName = "바뀔 State", GetOptions = "MNYS.YSDeveloperSettings.GetInputStateOptions"))
	FName InputState;

	UPROPERTY(EditDefaultsOnly, Category = "YS | Movement", meta = (DisplayName = "어빌리티 중 이동 차단"))
	bool bBlockMovementDuringAbility = false;

	UPROPERTY(EditDefaultsOnly, Category = "YS | Event Actions", meta = (Categories = "Event"))
	TMap<FGameplayTag, FYSEventPayload> EventActionMap;
	
protected :
	/** 재생할 노드의 출처. */
	UPROPERTY(EditDefaultsOnly, Category = "YS | Ability Playback", meta = (DisplayName = "플레이백 그래프"))
	TObjectPtr<UYSPlaybackGraphAsset> PlaybackGraph;

	/** 레거시 인라인 배열. 런타임은 읽지 않는다 — 그래프로 옮길 때만 쓴다. */
	UPROPERTY(EditDefaultsOnly, Category = "YS | Ability Playback", Instanced, meta = (DisplayName = "플레이백 (레거시 인라인)"))
	TArray<TObjectPtr<UYSAbilityPlaybackBase>> Playbacks;

private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<UYSAbilityPlaybackBase>> RuntimePlaybacks;

	/** 사본을 뜬 시점의 그래프 컴파일 일련번호. 에디터에서 다시 컴파일되면 사본을 새로 뜬다. */
	UPROPERTY(Transient)
	int32 CachedPlaybackSerial = INDEX_NONE;

	void EnsureRuntimePlaybacks();

protected:
	
	UPROPERTY(VisibleAnywhere)
	TWeakObjectPtr<UYSAbilityPlaybackBase> CurrentPlayback = nullptr;
	
	UPROPERTY()
	int32 CurrentPlaybackIndex;
	
	UPROPERTY(EditDefaultsOnly, Category = "YS | Buff", meta = (Categories = "Buff"))
	FGameplayTagContainer BuffTags;

	UPROPERTY(EditDefaultsOnly, meta = (RowType = "/Script/MNYS.YSSkillInfo"))
	FDataTableRowHandle SkillRow;
	
	UPROPERTY(EditDefaultsOnly, Category = "YS | Tag")
	FGameplayTagContainer WorldTagContainer;
	
	UPROPERTY(EditDefaultsOnly, Category = "YS | Movement", meta = (DisplayName = "어빌리티 동안 공중 정지"))
	bool bFreezeInAirDuringAbility = false;
	
protected :
	FYSGameplayAbility_RuntimeData RuntimeData;

	// 이번 활성에 어빌리티가 건 것의 해제 목록. 활성 시작과 EndAbility 끝에서 비운다.
	TArray<TFunction<void()>> EndCleanups;

	TSharedPtr<FYSAbilityHitContext> HitContext;
	TSharedPtr<FYSPlaybackContext> PlaybackContext;
	
	bool bIsInputAcceptable = false;

	// 커밋은 활성마다 한 번. '진입 시 커밋' 노드가 있으면 그 노드에서, 없으면 활성 때.
	bool bHasCommitted = false;

	// 이번 활성에 일이 있는 노드가 하나라도 켜졌는가. 활성 직후 체인이 일 없이 끝났는지 가른다.
	bool bPlaybackDidWork = false;

	UPROPERTY()
	UYSAT_Trace* TraceTask = nullptr;
	
private :
	// CancelAbility 가 도는 동안. 그 사이 다른 경로로 먼저 끝나도 취소로 정리한다.
	bool bCancelling = false;
};
