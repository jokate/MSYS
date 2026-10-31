// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "General/YSEnum.h"
#include "YSAbilitySystemComponent.generated.h"


struct FYSCharacterInfo;
struct FYSGrantedAbilityData;
class UYSGameplayAbility;

USTRUCT()
struct FYSCooldownEntry
{
	GENERATED_BODY()

	UPROPERTY()
	TSubclassOf<UGameplayAbility> AbilityClass;

	UPROPERTY()
	float EndTime = 0.f;
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGameplayTagStateChanged, const FGameplayTag&, Tag, bool, IsActive);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MNYS_API UYSAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UYSAbilitySystemComponent();
	static UYSAbilitySystemComponent* Get(AActor* Owner);
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual int32 HandleGameplayEvent(FGameplayTag EventTag, const FGameplayEventData* Payload) override;
	
	/** @return 활성 어빌리티가 받았거나 새로 켰으면 true */
	bool ProcessAbilityByInputPass(const TArray<FGameplayTag>& InputTags, EYSInputPhase InputPhase);
	void ApplyStatInitialization();
	void ApplyResourceRecharge(const FYSCharacterInfo* CharacterInfo);

	void ApplyDerivedStats(bool bRefillHp = false);

	// 최대 체력이 정해진 직후 한 번만 부른다. 다시 부르면 배율이 겹친다.
	void ApplyLevelHpScale();

	void GiveInitAbility();

	virtual void OnTagUpdated(const FGameplayTag& Tag, bool TagExists) override;
	
	void AllocateSkillToAbilityTag(const FGameplayTag& SkillTag, const TSubclassOf<UGameplayAbility> AbilityClass);
	
	bool IsOnCooldown(TSubclassOf<UGameplayAbility> Ability) const
	{
		return GetRemaining(Ability) > 0.f;
	}

	void AddAirUsage(const FGameplayAbilitySpecHandle& Ability)
	{
		AirUsedHandles.Add(Ability);
	}
	
	void ClearAirUsage();
	
	float GetRemaining(TSubclassOf<UGameplayAbility> Ability) const
	{
		for (const FYSCooldownEntry& Entry : CooldownEntries)
		{
			if (Entry.AbilityClass == Ability)
			{
				return FMath::Max(0.f, Entry.EndTime - GetWorld()->GetTimeSeconds());
			}
		}
		return 0.f;
	}
	
	void StartCoolDown(TSubclassOf<UGameplayAbility> Ability, float TargetToCooldownTime);

	AActor* GetLastJustAvoidInstigator() const { return LastJustAvoidInstigator.Get(); }
	
protected:
	virtual void OnGiveAbility(FGameplayAbilitySpec& AbilitySpec) override;
	virtual void OnRemoveAbility(FGameplayAbilitySpec& AbilitySpec) override;
	
	bool ProcessSkillActive(const FGameplayTag& InputTag);
	bool ProcessAlreadyActiveAbility(const FGameplayTag& InputTag, EYSInputPhase InputPhase);

	void OnEffectAppliedToSelf(UAbilitySystemComponent* Source, const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle Handle);

	/** 입력 바인딩을 플레이어 기준 한 번 저장한다. 스쿼드면 필드 폰이 세 슬롯을 모은다. */
	void SaveInputBindings();

public :
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

	UPROPERTY(EditDefaultsOnly, Category = "YS | Ability")
	TObjectPtr<class UYSAbilityDataAsset> GrantAbilityData;

	FOnGameplayTagStateChanged OnGameplayTagStateChanged;
	
	UPROPERTY()
	TArray<FYSCooldownEntry> CooldownEntries;
	
	UPROPERTY()
	FActiveGameplayEffectHandle ResourceRechargeHandle;
	
	UPROPERTY()
	TSet<FGameplayAbilitySpecHandle> AirUsedHandles;

private:
	TWeakObjectPtr<AActor> LastJustAvoidInstigator;

	// PossessedBy 는 교대마다 온다. 부여 · 스탯 초기화는 한 번만 해야 해서 끝난 것을 기억한다.
	bool bInitAbilityGiven = false;
	bool bStatInitialized = false;
};
