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
	GetWorld()->GetTimerManager().SetTimer(ComboTimerHandle, this, &UYSInputStateMachineComponent::ResetInputTags, InputProcessingTime, true);
	GetWorld()->GetTimerManager().SetTimer(InputTrimTimerHandle, this, &UYSInputStateMachineComponent::TrimInputHistory, InputExpirationTime, true);
	
	AbilitySystemComponent = UYSAbilitySystemComponent::Get(GetOwner());
	if (AbilitySystemComponent.IsValid() )
	{
		AbilitySystemComponent->OnGameplayTagStateChanged.AddUniqueDynamic(this, &UYSInputStateMachineComponent::OnTagUpdated);
	}
	
	AllCommandSequence = UYSDeveloperSettings::GetAllCommandSequences();
}

void UYSInputStateMachineComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	
}

void UYSInputStateMachineComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearTimer(ComboTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(InputTrimTimerHandle);
	
	Super::EndPlay(EndPlayReason);
}

void UYSInputStateMachineComponent::RecordInputHistory(const FGameplayTag& Tag, EYSInputPhase InputPhase)
{
	FYSInputHistory* InputHistory = InputHistories.FindByPredicate([&](const FYSInputHistory& History)
	{
		return History.InputTag == Tag && History.InputPhase == InputPhase;
	});
	
	if ( InputHistory == nullptr )
	{
		FYSInputHistory TmpInputHistory(Tag, InputPhase, GetWorld()->GetTimeSeconds());
		InputHistories.AddUnique(TmpInputHistory);
	}
	else
	{
		// 인풋 자체에 대해서 스태형 인풋으로 처리되는 개념 보다는 그냥 단순 입력에 대한 여부를 따질 것임.
		InputHistory->InputTime = GetWorld()->GetTimeSeconds();
	}
	
	UE_LOG(LogTemp, Log, TEXT("Recording Input : %s, Phase : %s"), *Tag.ToString(), *UEnum::GetValueAsString(InputPhase));
}

void UYSInputStateMachineComponent::TrimInputHistory()
{
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	InputHistories.RemoveAll([&](const FYSInputHistory& History)
	{
		return (CurrentTime - History.InputTime) > InputExpirationTime;
	});
}

int32 UYSInputStateMachineComponent::FindLiveHistoryIndex(const FGameplayTag& Tag, EYSInputPhase InputPhase) const
{
	const UWorld* World = GetWorld();

	if ( IsValid(World) == false )
	{
		return INDEX_NONE;
	}

	const float CurrentTime = World->GetTimeSeconds();

	return InputHistories.IndexOfByPredicate([&](const FYSInputHistory& History)
	{
		return History.InputTag == Tag
			&& History.InputPhase == InputPhase
			&& (CurrentTime - History.InputTime) <= InputExpirationTime;
	});
}

bool UYSInputStateMachineComponent::ContainsInputHistory(const FGameplayTag& Tag, EYSInputPhase InputPhase) const
{
	return FindLiveHistoryIndex(Tag, InputPhase) != INDEX_NONE;
}

void UYSInputStateMachineComponent::ConsumeInputHistory(const FGameplayTag& Tag, EYSInputPhase InputPhase)
{
	// 배열 원소의 참조를 Remove에 되먹이면 TArray::CheckAddress에 걸린다.
	// RemoveAll이 제자리 압축을 하는 동안 그 참조가 가리키는 값이 바뀌기 때문이다.
	// 인덱스로 잡아 RemoveAt 하면 별칭 문제가 없고, 소비 의미에도 맞다 —
	// Remove는 일치하는 항목을 전부 지우지만 소비는 가장 오래된 1건만 지워야 한다.
	const int32 FoundIndex = FindLiveHistoryIndex(Tag, InputPhase);

	if ( FoundIndex != INDEX_NONE )
	{
		InputHistories.RemoveAt(FoundIndex);
	}
}

// 콤보의 경우에는 State에 의존하지 않는 순수 인풋이기 때문에 사실상 의미가 없음.
FGameplayTag UYSInputStateMachineComponent::FindBestCombo(const FGameplayTag& Tag)
{
	FGameplayTag BestTag = Tag;
	ComboInputTags.Add(Tag);
	for ( const FYSCommandSequence* ComboSequence : AllCommandSequence )
	{
		if ( ComboSequence->IsSatisfiedCommand(ComboInputTags) == true )
		{
			BestTag = ComboSequence->Command;
			ComboInputTags.Empty();
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
	// 홀드 상태는 입력 차단 여부와 무관하게 항상 최신이어야 한다.
	// 차단 중에 뗀 것을 놓치면 그 입력은 영영 눌린 것으로 남는다.
	// 커맨드 승격 이전의 원본 태그를 쓴다 — 뗌도 같은 원본 태그로 들어오기 때문이다.
	switch ( InputPhase )
	{
	case EYSInputPhase::Pressed :
		HeldInputTags.Add(Tag);
		break;
	case EYSInputPhase::Released :
	case EYSInputPhase::Canceled :
		HeldInputTags.Remove(Tag);
		break;
	default :
		break;
	}

	if ( bIsInputBlocked )
		return;

	// 상태 해석 전에 원본 태그로 한 번 흘린다.
	// 여기서 나가야 State 조합을 등록하지 않아도 되고, 활성 콤보의
	// TryTransition 을 건드리지 않아 콤보가 끊기지 않는다.
	OnRawInputAccepted.Broadcast(Tag, InputPhase);

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

