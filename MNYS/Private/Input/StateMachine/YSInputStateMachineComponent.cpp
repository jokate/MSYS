// Fill out your copyright notice in the Description page of Project Settings.


#include "Input/StateMachine/YSInputStateMachineComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "YSAbilitySystemComponent.h"
#include "YSDeveloperSettings.h"
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

	TArray<UClass*> DerivedClasses;
	GetDerivedClasses(UYSInputStates::StaticClass(), DerivedClasses, true);
	for (UClass* StateClass : DerivedClasses)
	{
		UYSInputStates* State = NewObject<UYSInputStates>(this, StateClass);
		if (!IsValid(State))
			continue;
		State->InitState(GetOwner());
		InputStates.Emplace(State->GetStateType(), State);
	}

	TransitionState(EYSInputStatesType::Idle);
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UYSInputStateMachineComponent::ResetInputTags, InputProcessingTime, true);
	
	AbilitySystemComponent = UYSAbilitySystemComponent::Get(GetOwner());
	if (AbilitySystemComponent.IsValid() )
	{
		AbilitySystemComponent->OnGameplayTagStateChanged.AddUniqueDynamic(this, &UYSInputStateMachineComponent::OnTagUpdated);
	}
	
	AllCommandSequence = UYSDeveloperSettings::GetAllCommandSequences();
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
	for ( const FYSCommandSequence* ComboSequence : AllCommandSequence )
	{
		if ( ComboSequence->IsSatisfiedCommand(InputTags) == true )
		{
			BestTag = ComboSequence->Command;
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

