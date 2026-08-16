// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Data/YSDataStruct.h"
#include "General/YSEnum.h"
#include "General/YSMacros.h"
#include "General/YSStruct.h"
#include "Input/Combo/YSComboData.h"
#include "YSGameplayAbility.generated.h"

struct FYSPlaybackContext;
class UYSAbilityPlaybackBase;
class UYSPlaybackGraphAsset;
class ALevelSequenceActor;
class UYSAT_Trace;
struct FYSMontageSelector;
class UYSAbilityEventAction;
struct FYSComboTransition;

UENUM(BlueprintType)
enum class EYSAbilityType : uint8
{
	None,
	MeleeAttack UMETA(DisplayName = "근접 공격"),
	RangedAttack UMETA(DisplayName = "원거리 공격"),
	Dash UMETA(Displayname = "속도 적용"),
	NeedCameraDirect UMETA(DisplayName = "카메라 연출 필요"),
	NeedReady UMETA(DisplayName = "레디 필요"),
	UseResource UMETA(DisplayName = "자원 소모"),
	Hon_Only UMETA(DisplayName = "기사단장 혼 전용"),
};

struct FYSAbilityHitContext
{
	FYSAbilityHitContext() {};
	FYSAbilityHitContext(AActor* Instigator) : InstigatorActor(Instigator) {}
	
	void AddHitActor(AActor* HitActor) { HitActors.Add(HitActor); }
	void UpdateHitResult(UObject* HitObject, const FHitResult& HitResult)
	{
		for (auto It = RecentHitResults.CreateIterator(); It; ++It)
		{
			if (!It.Key().IsValid()) { It.RemoveCurrent(); }  // 유령 엔트리 정리
		}
		
		RecentHitResults.FindOrAdd(HitObject).Add(HitResult);
	}
	void RemoveTraceEntry(UObject* HitObject)
	{
		for (auto It = RecentHitResults.CreateIterator(); It; ++It)
		{
			if (!It.Key().IsValid()) { It.RemoveCurrent(); }  // 유령 엔트리 정리
		}
		
		RecentHitResults.Remove(HitObject);
	}
	
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

private : 
	TWeakObjectPtr<AActor> InstigatorActor;
	TSet<TWeakObjectPtr<AActor>> HitActors;
	TMap<TWeakObjectPtr<UObject>, TArray<FHitResult>> RecentHitResults;
};

USTRUCT()
struct FYSGameplayAbility_RuntimeData
{
	GENERATED_BODY()

public:
	// ── Accessors ─────────────────────────────────────────────────────────
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
	YS_ACCESSOR_REF(TSet<EYSAbilityType>, AbilityTypes);
	
	void NotifyPlaybackChainFinished();
	UYSAbilityPlaybackBase* GetPlaybackNode(int32 Index);

	/**
	 * 실제로 쓸 플레이백 배열.
	 *
	 * 그래프 에셋이 지정돼 있으면 그쪽 컴파일 산출물이, 아니면 레거시 배열이 나온다.
	 * 마이그레이션이 끝난 어빌리티와 안 끝난 어빌리티가 같은 코드로 돌아야 하기 때문이다.
	 */
	const TArray<TObjectPtr<UYSAbilityPlaybackBase>>& GetPlaybackArray() const;

#if WITH_EDITOR
	/** 마이그레이션 전용. 그래프 에셋을 무시하고 레거시 배열을 그대로 준다. */
	const TArray<TObjectPtr<UYSAbilityPlaybackBase>>& GetLegacyPlaybacks() const { return Playbacks; }
#endif
	
	const FGameplayEventData* GetEventData() const { return &CurrentEventData; }
	
	void AddRuntimeEffectSpecHandle(const FActiveGameplayEffectHandle& Handle) { RuntimeData.AddEffectSpecHandle(Handle); }
	
	void ActivePlayback(int32 Index);
	
	UYSAbilityPlaybackBase* GetCurrentPlayback() const { return CurrentPlayback.Get(); }
	int32 GetCurrentPlaybackIndex() const { return CurrentPlaybackIndex; }
	
	YS_BOOL_ACCESSOR(bIsInputAcceptable, InputAcceptable)
	
	const TSharedPtr<FYSAbilityHitContext>& GetHitContext() const { return HitContext; }
	const TSharedPtr<FYSPlaybackContext>& GetAbilityPlaybackContext() const { return PlaybackContext; }
	
	virtual bool CheckCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	UFUNCTION()
	virtual void OnMontagePlayed();

	UFUNCTION()
	void OnMontageInterrupted();
	
	virtual void SetupPlayBack(const FGameplayEventData* TriggerEventData);
	
	const FYSSkillInfo* GetSkillInfo() const;
	
public :
#if UE_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
private :
	UFUNCTION()
	void _ProcessEvent(FGameplayEventData Payload);
	
	void _PrepareForAbilityEvent();
	
	void _AddWorldTag();
	void _RemoveWorldTag();


public:
	UPROPERTY(EditDefaultsOnly, Category = "YS | Input Related", meta = (DisplayName = "바뀔 State"))
	EYSInputStatesType ChangeInputStateType;

	UPROPERTY(EditDefaultsOnly, Category = "YS | Movement", meta = (DisplayName = "어빌리티 중 이동 차단"))
	bool bBlockMovementDuringAbility = false;

	// 어빌리티 에디터 세팅
	UPROPERTY(EditDefaultsOnly, Category = "YS | Event Actions", meta = (Categories = "Event"))
	TMap<FGameplayTag, FYSEventPayload> EventActionMap;
	
protected :
	UPROPERTY(EditDefaultsOnly, Category = "YS | Ability Type")
	TSet<EYSAbilityType> AbilityTypes;
	
	/**
	 * 플레이백 그래프 에셋. 지정하면 아래 레거시 배열 대신 이쪽을 쓴다.
	 * 마이그레이션이 끝나면 레거시 배열은 비워도 된다.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "YS | Ability Playback", meta = (DisplayName = "플레이백 그래프"))
	TObjectPtr<UYSPlaybackGraphAsset> PlaybackGraph;

	/** 레거시 인라인 배열. 그래프 에셋이 없을 때만 쓰인다. */
	UPROPERTY(EditDefaultsOnly, Category = "YS | Ability Playback", Instanced, meta = (DisplayName = "플레이백 (레거시 인라인)"))
	TArray<TObjectPtr<UYSAbilityPlaybackBase>> Playbacks;
	
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
	
protected :
	FYSGameplayAbility_RuntimeData RuntimeData;
	
	TSharedPtr<FYSAbilityHitContext> HitContext;
	TSharedPtr<FYSPlaybackContext> PlaybackContext;
	
	UPROPERTY()
	TObjectPtr<ALevelSequenceActor> ActiveSequenceActor = nullptr;
	
	// 인풋에 대한 어셉팅 여부
	bool bIsInputAcceptable = false;

	// NeedReady 어빌리티의 커밋 지연용. 확정 노드 진입 시 한 번만 커밋한다.
	bool bHasCommitted = false;

	UPROPERTY()
	UYSAT_Trace* TraceTask = nullptr;
};
