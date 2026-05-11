// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "General/YSEnum.h"
#include "General/YSMacros.h"
#include "Input/Combo/YSComboData.h"
#include "MontageSelector/YSMontageSelector.h"
#include "StructUtils/InstancedStruct.h"
#include "YSGameplayAbility.generated.h"

class UYSAT_Trace;
struct FYSMontageSelector;
class UYSAbilityEventAction;
struct FYSComboTransition;

USTRUCT()
struct FYSGameplayAbility_RuntimeData
{
	GENERATED_BODY()

public:
	// ── Accessors ─────────────────────────────────────────────────────────
	YS_GETTER(const FYSComboTransition*, PendingTransition)
	YS_SETTER(const FYSComboTransition*, PendingTransition)
	YS_BOOL_ACCESSOR(bIsInputAcceptable, InputAcceptable)
	YS_BOOL_ACCESSOR(bIsChainedAbility,  ChainedAbility)

	FORCEINLINE bool IsPendingTransition() const { return PendingTransition != nullptr; }

	void ResetData()
	{
		PendingTransition  = nullptr;
		bIsInputAcceptable = false;
		bIsChainedAbility  = false;
	}

private:
	// 다음에 펜딩 되어야 하는 트랜지션
	const FYSComboTransition* PendingTransition = nullptr;

	// 인풋에 대한 어셉팅 여부
	bool bIsInputAcceptable = false;

	// 어빌리티에 대해서 체이닝이 성공한 경우
	bool bIsChainedAbility = false;
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
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	bool TryTransition(const FGameplayTag& InputGameplayTag);

	UFUNCTION()
	void OnTraceComplete(const TArray<FHitResult>& HitResults);
	
	YS_ACCESSOR(UYSAT_Trace*, TraceTask);
	
protected:
	UFUNCTION()
	void OnMontagePlayed();

	UFUNCTION()
	void OnMontageInterrupted();
	
private :
	UFUNCTION()
	void _ProcessEvent(FGameplayEventData Payload);
	
	void _PrepareForAbilityEvent();

	void _SetupPlayMontage();

	void _ReleaseMotionWarp() const;
	
public:
	UPROPERTY(EditDefaultsOnly, Category = "YS | Montage To Play", meta = (DisplayName = "재생할 몽타주 정보", BaseStruct = "/Script/MNYS.YSMontageSelector", ExcludeBaseStruct))
	FInstancedStruct MontageSelector;

	UPROPERTY(EditDefaultsOnly, Category = "YS | Input Related", meta = (DisplayName = "인풋에 따른 반응"))
	TArray<FYSComboTransition> TransitionsByInput;

	UPROPERTY(EditDefaultsOnly, Category = "YS | Input Related", meta = (DisplayName = "바뀔 State"))
	EYSInputStatesType ChangeInputStateType;

	UPROPERTY(EditDefaultsOnly, Category = "YS | Movement", meta = (DisplayName = "어빌리티 중 이동 차단"))
	bool bBlockMovementDuringAbility = false;

	// 어빌리티 에디터 세팅
	UPROPERTY(EditDefaultsOnly, Category = "YS | Event Actions")
	TMap<FGameplayTag, FYSEventPayload> EventActionMap;

	
private:
	FYSGameplayAbility_RuntimeData RuntimeData;

	UPROPERTY()
	UYSAT_Trace* TraceTask = nullptr;
};
