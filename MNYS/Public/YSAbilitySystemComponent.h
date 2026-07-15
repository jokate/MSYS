// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "YSAbilitySystemComponent.generated.h"


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
	void ProcessAbilityByInputPass(const FGameplayTag& InputTag);
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
	bool ProcessAlreadyActiveAbility(const FGameplayTag& InputTag);
	
public :
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

	UPROPERTY(EditDefaultsOnly, Category = "YS | Ability")
	TObjectPtr<class UYSAbilityDataAsset> GrantAbilityData;

	FOnGameplayTagStateChanged OnGameplayTagStateChanged;
	
	UPROPERTY()
	TArray<FYSCooldownEntry> CooldownEntries;
};
