// Fill out your copyright notice in the Description page of Project Settings.


#include "Input/StateMachine/YSInputStateMachineComponent.h"

#include "Input/StateMachine/YSInputStates.h"


UYSInputStateMachineComponent* UYSInputStateMachineComponent::Get(AActor* Owner)
{
	return Owner->FindComponentByClass<UYSInputStateMachineComponent>();
}

UYSInputStateMachineComponent::UYSInputStateMachineComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UYSInputStateMachineComponent::BeginPlay()
{
	Super::BeginPlay();

	AddState<UYSAttackState>();
	AddState<UYSIdleState>();
	AddState<UYSDodgeState>();
	AddState<UYSFallingState>();
	AddState<UYSReadyState>();
	AddState<UYSJustAvoidState>();
	AddState<UYSSkillState>();
}

void UYSInputStateMachineComponent::AcceptInput(const FGameplayTag& Tag)
{
	if ( IsValid(CurrentInputState))
	{
		CurrentInputState->ProcessInput(Tag);
	}
}

void UYSInputStateMachineComponent::TransitionState(EYSInputStatesType NewInputState)
{
	if ( IsValid(CurrentInputState) && CurrentInputState->IsEnableTransition(NewInputState))
	{
		UYSInputStates** NextState = InputStates.Find(NewInputState);

		if ( NextState == nullptr )
		{
			return;
		}

		CurrentInputState = *NextState;
	}
}


template <typename T>
void UYSInputStateMachineComponent::AddState()
{
	T* NewState = NewObject<T>(this);

	if ( IsValid(NewState) == false )
	{
		return;
	}

	UYSInputStates* CreatedState = Cast<UYSInputStates>(NewState);
	if ( IsValid(CreatedState) == false )
		return;

	CreatedState->InitState(GetOwner());
	InputStates.Emplace(CreatedState->GetStateType(), CreatedState);
}

