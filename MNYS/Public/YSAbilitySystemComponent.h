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
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnGiveAbility(FGameplayAbilitySpec& AbilitySpec) override;
	virtual void OnRemoveAbility(FGameplayAbilitySpec& AbilitySpec) override;
	
	bool ProcessSkill(const FGameplayTag& InputTag);
	void ProcessCombo(const FGameplayTag& InputTag);
	void ResetInputTags()
	{
		InputTags.Empty();
	}
	
public :
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;
	
	UPROPERTY()
	FTimerHandle TimerHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FGameplayTag> InputTags;

	// 해당 값은 조작감에 따라서 처리되기로 합시다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float InputProcessingTime = 0.2f;
};
