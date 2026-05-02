// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "YSAbilitySystemComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MNYS_API UYSAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UYSAbilitySystemComponent();
	void ProcessAbilityByInputPass(const FGameplayTag& InputTag);
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void OnGiveAbility(FGameplayAbilitySpec& AbilitySpec) override;
	virtual void OnRemoveAbility(FGameplayAbilitySpec& AbilitySpec) override;

public :
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> AbilitySpecs;

	UPROPERTY()
	FTimerHandle TimerHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FGameplayTag> InputTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float InputProcessingTime;
};
