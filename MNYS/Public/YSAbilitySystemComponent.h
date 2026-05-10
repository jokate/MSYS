// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "YSAbilitySystemComponent.generated.h"


struct FYSGrantedAbilityData;
class UYSGameplayAbility;

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
	void ProcessAbilityByInputPass(const FGameplayTag& InputTag);
	void GiveInitAbility();

	virtual void OnTagUpdated(const FGameplayTag& Tag, bool TagExists) override;
	
protected:
	virtual void OnGiveAbility(FGameplayAbilitySpec& AbilitySpec) override;
	virtual void OnRemoveAbility(FGameplayAbilitySpec& AbilitySpec) override;
	
	bool ProcessSkillActive(const FGameplayTag& InputTag);
	bool ProcessAlreadyActiveAbility(const FGameplayTag& InputTag);
	
public :
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

	UPROPERTY(EditDefaultsOnly, Category = "YS | Ability")
	TArray<FYSGrantedAbilityData> GrantedAbilities;

	FOnGameplayTagStateChanged OnGameplayTagStateChanged;
};
