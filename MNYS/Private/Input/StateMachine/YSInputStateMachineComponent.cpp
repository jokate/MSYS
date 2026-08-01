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

	AddStateStack(EYSInputStatesType::Idle);
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
			InputTags.Empty();
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

void UYSInputStateMachineComponent::AcceptInput(const FGameplayTag& Tag, EYSInputPhase InputPhase)
{
	if ( bIsInputBlocked )
		return;

	// 커맨드 버퍼에는 "누름"만 먹인다.
	// 뗌까지 InputTags에 쌓이면 FYSCommandSequence::IsSatisfiedCommand의 순서 판정이 깨진다.
	const FGameplayTag RetTag = (InputPhase == EYSInputPhase::Pressed) ? FindBestCombo(Tag) : Tag;

	if ( IsValid(CurrentInputState))
    {
        CurrentInputState->ProcessInput(RetTag, InputPhase);
    }
}

void UYSInputStateMachineComponent::TransitionState(EYSInputStatesType NewInputState)
{
	if ( IsValid(CurrentInputState) && CurrentInputState->IsEnableTransition(NewInputState) == false )
	{
		return;
	}
	
	UYSInputStates** NextState = InputStates.Find(NewInputState);

	if ( NextState == nullptr )
	{
		return;
	}

	CurrentInputState = *NextState;
}

void UYSInputStateMachineComponent::AddStateStack(EYSInputStatesType State)
{
	// 만약 State Stack에 있는 경우 최상단으로 올려준다.
	if ( InputStateRequests.Contains(State))
	{
		InputStateRequests.Remove(State);
	}
	
	InputStateRequests.Add(State);	
	
	// 근데 이렇게 State 전환이 정상적이지 않다면 어쩌지?
	TransitionState(State);
}

void UYSInputStateMachineComponent::RemoveStateStack(EYSInputStatesType State)
{
	// 유일성을 보장한다. (Idle이 아닌 경우에는 제거 가능)
	if (State != EYSInputStatesType::Idle)
	{
		InputStateRequests.Remove(State);
	}
	
	if ( InputStateRequests.IsEmpty() )
		return;
	
	TransitionState(InputStateRequests.Last());
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

