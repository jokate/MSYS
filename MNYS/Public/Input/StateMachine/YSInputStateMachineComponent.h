// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "YSEnum.h"
#include "Components/ActorComponent.h"
#include "YSInputStateMachineComponent.generated.h"


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
	virtual void TransitionState(EYSInputStatesType NewInputState);
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	template<typename T>
	void AddState();

public : 
	UPROPERTY()
	TObjectPtr<UYSInputStates> CurrentInputState;

	UPROPERTY()
	TMap<EYSInputStatesType, UYSInputStates*> InputStates;
};
