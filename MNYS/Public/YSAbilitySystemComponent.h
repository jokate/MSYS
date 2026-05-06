// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "YSAbilitySystemComponent.generated.h"


class UYSGameplayAbility;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MNYS_API UYSAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UYSAbilitySystemComponent();
	virtual void BeginPlay() override;
	void ProcessAbilityByInputPass(const FGameplayTag& InputTag);
	void GiveAbilities();
protected:
	virtual void OnGiveAbility(FGameplayAbilitySpec& AbilitySpec) override;
	virtual void OnRemoveAbility(FGameplayAbilitySpec& AbilitySpec) override;
	
	bool ProcessSkillActive(const FGameplayTag& InputTag);
	bool ProcessAlreadyActiveAbility(const FGameplayTag& InputTag);
	
public :
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

	UPROPERTY(EditDefaultsOnly)
	TArray<TSubclassOf<UYSGameplayAbility>> AbilityClasses;
};
