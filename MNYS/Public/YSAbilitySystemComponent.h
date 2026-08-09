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
	// Sets default values for this component's properties
	UYSAbilitySystemComponent();
	static UYSAbilitySystemComponent* Get(AActor* Owner);
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	void ProcessAbilityByInputPass(const FGameplayTag& InputTag, EYSInputPhase InputPhase);
	void ApplyStatInitialization();
	void ApplyResourceRecharge(const FYSCharacterInfo* CharacterInfo);

	/**
	 * StatXXX 어트리뷰트를 기준으로 파생 수치(MaxHp, 공격력, 이동속도, 태그 게이지 효율)를 재산출한다.
	 * 초기화 시점뿐 아니라 각성 제단/봉인 해방 등으로 원본 스탯이 변한 뒤에도 호출한다.
	 * @param bRefillHp 파생 직후 Hp 를 MaxHp 로 채울지 여부. 런 중 재산출 시에는 false 로 둔다.
	 */
	void ApplyDerivedStats(bool bRefillHp = false);

	void GiveInitAbility();

	virtual void OnTagUpdated(const FGameplayTag& Tag, bool TagExists) override;
	void AllocateSkillToAbilityTag(const FGameplayTag& SkillTag, const TSubclassOf<UGameplayAbility> AbilityClass);
	
	bool IsOnCooldown(TSubclassOf<UGameplayAbility> Ability) const
	{
		return GetRemaining(Ability) > 0.f;
	}

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
	
protected:
	virtual void OnGiveAbility(FGameplayAbilitySpec& AbilitySpec) override;
	virtual void OnRemoveAbility(FGameplayAbilitySpec& AbilitySpec) override;
	
	bool ProcessSkillActive(const FGameplayTag& InputTag);
	bool ProcessAlreadyActiveAbility(const FGameplayTag& InputTag, EYSInputPhase InputPhase);
	
	
	
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
};
