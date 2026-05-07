// Fill out your copyright notice in the Description page of Project Settings.


#include "Input/StateMachine/YSInputStateMachineComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "YSAbilitySystemComponent.h"
#include "General/YSGameplayTag.h"
#include "Input/Combo/YSComboData.h"
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

	TransitionState(EYSInputStatesType::Idle);
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UYSInputStateMachineComponent::ResetInputTags, InputProcessingTime, true);
	
	AbilitySystemComponent = UYSAbilitySystemComponent::Get(GetOwner());
	if (AbilitySystemComponent.IsValid() )
	{
		AbilitySystemComponent->OnGameplayTagStateChanged.AddUniqueDynamic(this, &UYSInputStateMachineComponent::OnTagUpdated);
	}
}

void UYSInputStateMachineComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	
	Super::EndPlay(EndPlayReason);
}

FGameplayTag UYSInputStateMachineComponent::FindBestCombo(const FGameplayTag& Tag)
{
	FGameplayTag BestTag = Tag;
	InputTags.Add(Tag);
	for ( const FYSComboSequence* ComboSequence : AllComboSequence )
	{
		if ( ComboSequence->IsSatisfiedCombo(InputTags) == true )
		{
			BestTag = ComboSequence->Combo;
			break;
		}
	}

	return BestTag;
}

void UYSInputStateMachineComponent::OnTagUpdated(const FGameplayTag& Tag, bool bActive)
{
	if ( Tag == YSTags::BlockInput )
	{
		bIsInputBlocked = bActive;
	}
}

void UYSInputStateMachineComponent::AcceptInput(const FGameplayTag& Tag)
{
	if ( bIsInputBlocked )
		return;
	
	FGameplayTag RetTag = FindBestCombo(Tag);
	
	if ( IsValid(CurrentInputState))
    {
        CurrentInputState->ProcessInput(RetTag);
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
	else
	{
		CurrentInputState = *InputStates.Find(NewInputState);
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

