// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "General/YSEnum.h"
#include "General/YSMacros.h"
#include "Input/Combo/YSComboData.h"
#include "YSGameplayAbility.generated.h"

struct FYSPlaybackContext;
class UYSAbilityPlaybackBase;
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
};

struct FYSAbilityHitContext
{
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

	bool TryTransition(const FGameplayTag& InputGameplayTag);
	
	YS_ACCESSOR(UYSAT_Trace*, TraceTask)
	YS_ACCESSOR_REF(TSet<EYSAbilityType>, AbilityTypes);
	
	void NotifyPlaybackChainFinished();
	UYSAbilityPlaybackBase* GetPlaybackNode(int32 Index);
	
	const FGameplayEventData* GetEventData() const { return &CurrentEventData; }
	
	void AddRuntimeEffectSpecHandle(const FActiveGameplayEffectHandle& Handle) { RuntimeData.AddEffectSpecHandle(Handle); }
	
	void ActivePlayback(int32 Index, const FYSPlaybackContext& Context);
	
	UYSAbilityPlaybackBase* GetCurrentPlayback() const { return CurrentPlayback.Get(); }
	
	YS_BOOL_ACCESSOR(bIsInputAcceptable, InputAcceptable)
	
	TSharedPtr<FYSAbilityHitContext> GetHitContext() const { return HitContext; }
	
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	UFUNCTION()
	virtual void OnMontagePlayed();

	UFUNCTION()
	void OnMontageInterrupted();
	
	virtual void SetupPlayBack(const FGameplayEventData* TriggerEventData);
	
public :
#if UE_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
private :
	UFUNCTION()
	void _ProcessEvent(FGameplayEventData Payload);
	
	void _PrepareForAbilityEvent();

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
	
	UPROPERTY(EditDefaultsOnly, Category = "YS | Ability Playback", Instanced)
	TArray<UYSAbilityPlaybackBase*> Playbacks;
	
	UPROPERTY(VisibleAnywhere)
	TWeakObjectPtr<UYSAbilityPlaybackBase> CurrentPlayback = nullptr;
	
	UPROPERTY(EditDefaultsOnly, Category = "YS | Buff", meta = (Categories = "Buff"))
	FGameplayTagContainer BuffTags;
	
private:
	FYSGameplayAbility_RuntimeData RuntimeData;
	
	TSharedPtr<FYSAbilityHitContext> HitContext;
	
	UPROPERTY()
	TObjectPtr<ALevelSequenceActor> ActiveSequenceActor = nullptr;
	
	// 인풋에 대한 어셉팅 여부
	bool bIsInputAcceptable = false;

	UPROPERTY()
	UYSAT_Trace* TraceTask = nullptr;
};
