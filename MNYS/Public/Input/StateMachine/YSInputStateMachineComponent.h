// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "General/YSEnum.h"
#include "YSInputStateMachineComponent.generated.h"


class UYSAbilitySystemComponent;
struct FYSCommandSequence;
class UYSInputStates;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MNYS_API UYSInputStateMachineComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	static UYSInputStateMachineComponent* Get(AActor* Owner);
	UYSInputStateMachineComponent();
	virtual void AcceptInput(const FGameplayTag& Tag);
	
	virtual void AddStateStack(EYSInputStatesType State);
	virtual void RemoveStateStack(EYSInputStatesType State);
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
protected:
	void ResetInputTags()
	{
		InputTags.Empty();
	}
	
	virtual void TransitionState(EYSInputStatesType NewInputState);
	
	// Called when the game starts

	template<typename T>
	void AddState();

	FGameplayTag FindBestCombo(const FGameplayTag& Tag);

	UFUNCTION()
	void OnTagUpdated(const FGameplayTag& Tag, bool bActive);

public :
	// 현재 InputState에 대한 요청들을 담아둡니다. 만약 Transition이 불가능하더라도 요청은 남습니다.
	// 만약 제거 될 때 최상단의 State로 변경됩니다.
	UPROPERTY(VisibleAnywhere)
	TArray<EYSInputStatesType> InputStateRequests;
	
	UPROPERTY()
	TObjectPtr<UYSInputStates> CurrentInputState;

	UPROPERTY()
	TMap<EYSInputStatesType, UYSInputStates*> InputStates;

protected:
	UPROPERTY()
	FTimerHandle TimerHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FGameplayTag> InputTags;

	// 해당 값은 조작감에 따라서 처리되기로 합시다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float InputProcessingTime = 0.33f;

private :
	TArray<const FYSCommandSequence*> AllCommandSequence;

	bool bIsInputBlocked;
	TWeakObjectPtr<UYSAbilitySystemComponent> AbilitySystemComponent;
};
