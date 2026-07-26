// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "Data/YSDataStruct.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "YSSquadSubsystem.generated.h"

/**
 * 
 */

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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FYSOnTagGaugeChanged, float, NewGauge);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FYSOnActiveSlotChanged, int32, OldSlot, int32, NewSlot);


UCLASS()
class MNYS_API UYSSquadSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
	
public :
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

	/** 현재 필드 캐릭터의 ASC */
	UFUNCTION(BlueprintPure, Category = "YS | Squad")
	UYSAbilitySystemComponent* GetActiveASC() const;
	
	UFUNCTION(BlueprintCallable, Category = "YS | Squad")
	void AddTagGauge(float Amount);

	/** 게이지를 소모한다. 부족하면 false를 반환하고 아무것도 소모하지 않는다. */
	UFUNCTION(BlueprintCallable, Category = "YS | Squad")
	bool TryConsumeGauge(float Cost);

	UFUNCTION(BlueprintPure, Category = "YS | Squad")
	float GetTagGauge() const { return TagGauge; }
	
	UPROPERTY(BlueprintAssignable, Category = "YS | Squad")
	FYSOnTagGaugeChanged OnTagGaugeChanged;

	UPROPERTY(BlueprintAssignable, Category = "YS | Squad")
	FYSOnActiveSlotChanged OnActiveSlotChanged;

protected:
	void PerformTag(int32 NewSlot, bool bSwitchAttack);
	
	void RefreshBackupPassives();
	
	void SetPawnActive(APawn* Pawn, bool bActive);

	UYSAbilitySystemComponent* GetASC(int32 SlotIndex) const;;

protected : 
	UPROPERTY(EditDefaultsOnly, Category = "YS | Squad")
	float TagCooldown = 8.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "YS | Squad")
	float MaxTagGauge = 3.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "YS | Squad")
	float SwitchAttackCost = 1.0f;
	
private:
	UPROPERTY()
	TArray<FYSSquadSlot> Slots;

	UPROPERTY()
	int32 ActiveSlot = 0;

	UPROPERTY()
	float TagGauge = 0.f;

	FTimerHandle TagCooldownTimer;
};