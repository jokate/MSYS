// Fill out your copyright notice in the Description page of Project Settings.


#include "Input/StateMachine/YSInputStateMachineComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "YSAbilitySystemComponent.h"
#include "YSDeveloperSettings.h"
#include "GameplayEffect.h"
#include "Ability/GameplayEffectComponents/YSInputStateGEComponent.h"
#include "General/YSGameplayTag.h"
#include "General/YSDefine.h"
#include "Input/Combo/YSComboData.h"
#include "Input/StateMachine/YSInputStates.h"


UYSInputStateMachineComponent* UYSInputStateMachineComponent::Get(AActor* Owner)
{
	if ( IsValid(Owner) == false )
	{
		return nullptr;
	}

	return Owner->FindComponentByClass<UYSInputStateMachineComponent>();
}

UYSInputStateMachineComponent::UYSInputStateMachineComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}


void UYSInputStateMachineComponent::BeginPlay()
{
	Super::BeginPlay();

	const UDataTable* StateTable = GetDefault<UYSDeveloperSettings>()->GetInputStateTable();

	if ( IsValid(StateTable) == false )
	{
		UE_LOG(LogTemp, Warning, TEXT("[InputState] 입력 상태 정의 표가 비었거나 행 구조가 다르다. YS Developer Settings 에서 채워야 한다."));
	}
	else
	{
		for ( const TPair<FName, uint8*>& Pair : StateTable->GetRowMap() )
		{
			const FYSInputStateDefinition& Definition = *reinterpret_cast<const FYSInputStateDefinition*>(Pair.Value);

			// 표가 고른 GE 가 다른 상태를 걸면 어빌리티는 맞게 골라도 엉뚱한 상태가 된다.
			const UGameplayEffect* StateEffect = Definition.StateEffect ? Definition.StateEffect->GetDefaultObject<UGameplayEffect>() : nullptr;
			const UYSInputStateGEComponent* StateComponent = StateEffect ? StateEffect->FindComponent<UYSInputStateGEComponent>() : nullptr;

			if ( StateEffect != nullptr && (StateComponent == nullptr || StateComponent->TargetToChange != Pair.Key) )
			{
				UE_LOG(LogTemp, Warning, TEXT("[InputState] 행 %s 의 GE %s 가 이 상태를 걸지 않는다 (GE 의 상태: %s)."),
					*Pair.Key.ToString(), *GetNameSafe(Definition.StateEffect), StateComponent ? *StateComponent->TargetToChange.ToString() : TEXT("컴포넌트 없음"));
			}

			UYSInputStates* State = NewObject<UYSInputStates>(this);
			State->InitState(GetOwner(), Pair.Key, Definition);
			InputStates.Emplace(Pair.Key, State);
		}
	}

	AddStateStack(BaseInputState);
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
	
	TrimCommandInput();
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
		InputHistory->InputTime = GetWorld()->GetTimeSeconds();
	}
	
	UE_LOG(LogTemp, Log, TEXT("Recording Input : %s, Phase : %s"), *Tag.ToString(), *UEnum::GetValueAsString(InputPhase));
}

void UYSInputStateMachineComponent::TrimInputHistory()
{
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	InputHistories.RemoveAll([&](const FYSInputHistory& History)
	{
		const bool bExpired = (CurrentTime - History.InputTime) > InputExpirationTime;
		if ( bExpired )
		{
			UE_LOG(LogYSMeasure, Verbose, TEXT("[Input] t=%.3f Expire Tag=%s Age=%.3f Expire=%.3f"), CurrentTime, *History.InputTag.ToString(), CurrentTime - History.InputTime, InputExpirationTime);
		}
		return bExpired;
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
	const int32 FoundIndex = FindLiveHistoryIndex(Tag, InputPhase);

	if ( FoundIndex != INDEX_NONE )
	{
		UE_LOG(LogYSMeasure, Verbose, TEXT("[Input] t=%.3f Consume Tag=%s Age=%.3f Expire=%.3f"),
			GetWorld()->GetTimeSeconds(), *Tag.ToString(), GetWorld()->GetTimeSeconds() - InputHistories[FoundIndex].InputTime, InputExpirationTime);
		InputHistories.RemoveAt(FoundIndex);
	}
}

FGameplayTag UYSInputStateMachineComponent::FindBestCombo(const FGameplayTag& Tag)
{
	FYSTagHistory ComboInputTag = FYSTagHistory(Tag, GetWorld()->GetTimeSeconds());
	ComboInputTags.Add(ComboInputTag);

	// 첫 일치가 아니라 가장 긴 일치를 고른다. 접미 매칭에서는 짧은 커맨드가 긴 커맨드의 꼬리에 항상 포함되므로 행 순서에 맡기면 긴 커맨드가 나오지 않는다.
	const FYSCommandSequence* BestSequence = nullptr;

	for ( const FYSCommandSequence* ComboSequence : AllCommandSequence )
	{
		if ( ComboSequence->IsSatisfiedCommand(ComboInputTags) == false )
		{
			continue;
		}

		if ( BestSequence == nullptr || ComboSequence->CommandSequence.Num() > BestSequence->CommandSequence.Num() )
		{
			BestSequence = ComboSequence;
		}
	}

	if ( BestSequence == nullptr )
	{
		return Tag;
	}

	ComboInputTags.Empty();
	return BestSequence->Command;
}

void UYSInputStateMachineComponent::OnTagUpdated(const FGameplayTag& Tag, bool bActive)
{
	if ( Tag == YSTags::BlockInput )
	{
		bIsInputBlocked = bActive;
	}
}


void UYSInputStateMachineComponent::TrimCommandInput()
{
	// 커맨드는 입력이 들어온 경우에만 처리한다. 타이머 주기 검사는 인식이 더뎌 조작감이 나빴다.
	if ( ComboInputTags.Num() == 0 )
	{
		return;
	}

	for ( int32 Index = ComboInputTags.Num() - 1; Index >= 0; --Index )
	{
		const FYSTagHistory& History = ComboInputTags[Index];
		if ( (GetWorld()->GetTimeSeconds() - History.InputTime) > InputExpirationTime )
		{
			ComboInputTags.RemoveAt(Index);
		}
	}
}

void UYSInputStateMachineComponent::AcceptInput(const FGameplayTag& Tag, EYSInputPhase InputPhase)
{
	// 홀드 상태는 입력 차단 여부와 무관하게 항상 최신이어야 한다 (차단 중 뗌을 놓치면 영영 눌린 것으로 남는다). 뗌도 원본 태그로 들어오므로 커맨드 승격 이전의 원본 태그를 쓴다.
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

	if ( HoldInputIfRequested(Tag, InputPhase) )
		return;

	DispatchInput(Tag, InputPhase);
}

bool UYSInputStateMachineComponent::HoldInputIfRequested(const FGameplayTag& Tag, EYSInputPhase InputPhase)
{
	InputHoldRequesters.RemoveAll([](const TWeakObjectPtr<const UObject>& Requester) { return Requester.IsValid() == false; });

	if ( InputHoldRequesters.IsEmpty() )
	{
		FlushHeldPress();
		return false;
	}

	if ( InputPhase == EYSInputPhase::Pressed )
	{
		// 첫 누름 하나만 남긴다 (처음 입력 하나로 파생이 갈린다).
		if ( HeldPress.IsValid() == false )
		{
			HeldPress = Tag;
			bHeldPressReleased = false;
		}
		return true;
	}

	if ( HeldPress.IsValid() && Tag == HeldPress && (InputPhase == EYSInputPhase::Released || InputPhase == EYSInputPhase::Canceled) )
	{
		bHeldPressReleased = true;
		return true;
	}

	// 보관 전에 누른 키의 뗌은 그대로 흘린다. 막으면 그 입력이 영영 눌린 것으로 남는다.
	return false;
}

void UYSInputStateMachineComponent::PushInputHold(const UObject* Requester)
{
	if ( Requester == nullptr )
		return;

	InputHoldRequesters.AddUnique(Requester);
}

void UYSInputStateMachineComponent::PopInputHold(const UObject* Requester)
{
	InputHoldRequesters.Remove(Requester);
	InputHoldRequesters.RemoveAll([](const TWeakObjectPtr<const UObject>& Other) { return Other.IsValid() == false; });

	if ( InputHoldRequesters.IsEmpty() )
	{
		FlushHeldPress();
	}
}

void UYSInputStateMachineComponent::ClaimRawInput(const FGameplayTag& Tag, const UObject* Claimer)
{
	if ( Tag.IsValid() == false || Claimer == nullptr )
		return;

	RawInputClaims.AddUnique(Tag, Claimer);
}

void UYSInputStateMachineComponent::ReleaseRawInputs(const UObject* Claimer)
{
	for ( auto It = RawInputClaims.CreateIterator(); It; ++It )
	{
		if ( It.Value().IsValid() == false || It.Value().Get() == Claimer )
		{
			It.RemoveCurrent();
		}
	}
}

bool UYSInputStateMachineComponent::IsRawInputClaimed(const FGameplayTag& Tag) const
{
	for ( auto It = RawInputClaims.CreateConstKeyIterator(Tag); It; ++It )
	{
		if ( It.Value().IsValid() )
			return true;
	}

	return false;
}

void UYSInputStateMachineComponent::FlushHeldPress()
{
	if ( HeldPress.IsValid() == false )
		return;

	const FGameplayTag Press = HeldPress;
	const bool bReleased = bHeldPressReleased;
	HeldPress = FGameplayTag();
	bHeldPressReleased = false;

	if ( bIsInputBlocked )
		return;

	DispatchInput(Press, EYSInputPhase::Pressed);

	if ( bReleased )
	{
		DispatchInput(Press, EYSInputPhase::Released);
	}
}

void UYSInputStateMachineComponent::DispatchInput(const FGameplayTag& Tag, EYSInputPhase InputPhase)
{
	OnRawInputAccepted.Broadcast(Tag, InputPhase);

	if ( IsRawInputClaimed(Tag) )
		return;

	// 커맨드 버퍼에는 "누름"만 먹인다. 뗌까지 InputTags 에 쌓이면 FYSCommandSequence::IsSatisfiedCommand 의 순서 판정이 깨진다.
	const FGameplayTag RetTag = (InputPhase == EYSInputPhase::Pressed) ? FindBestCombo(Tag) : Tag;

	if ( IsValid(CurrentInputState))
    {
        CurrentInputState->ProcessInput(RetTag, InputPhase);
    }
}

UYSInputStates* UYSInputStateMachineComponent::FindState(FName State) const
{
	const TObjectPtr<UYSInputStates>* Found = InputStates.Find(State);
	return Found != nullptr ? Found->Get() : nullptr;
}

FActiveGameplayEffectHandle UYSInputStateMachineComponent::ApplyStateEffect(FName State, const UObject* SourceObject) const
{
	if ( State.IsNone() )
	{
		return FActiveGameplayEffectHandle();
	}

	const FYSInputStateDefinition* Definition = GetDefault<UYSDeveloperSettings>()->FindInputStateDefinition(State);

	if ( Definition == nullptr || Definition->StateEffect == nullptr )
	{
		UE_LOG(LogTemp, Warning, TEXT("[InputState] 상태 %s 의 GE 가 정의 표에 없다."), *State.ToString());
		return FActiveGameplayEffectHandle();
	}

	// BeginPlay 전에도 불린다(스폰 직후 낙하). 캐시가 아니라 주인에게서 찾는다.
	UYSAbilitySystemComponent* ASC = UYSAbilitySystemComponent::Get(GetOwner());

	if ( IsValid(ASC) == false )
	{
		return FActiveGameplayEffectHandle();
	}

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	Context.AddSourceObject(SourceObject);

	const FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(Definition->StateEffect, 1.f, Context);
	return Spec.IsValid() ? ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get()) : FActiveGameplayEffectHandle();
}

void UYSInputStateMachineComponent::TransitionState(FName NewInputState)
{
	UYSInputStates* NextState = FindState(NewInputState);

	if ( NextState == nullptr )
	{
		return;
	}

	CurrentInputState = NextState;
}

void UYSInputStateMachineComponent::AddStateStack(FName State)
{
	// None 은 "바꾸지 않음" 이다. 맨 위에 쌓이면 전이에 실패해 이전 상태 객체가 그대로 남는다.
	if ( State.IsNone() )
	{
		return;
	}

	if ( InputStateRequests.Contains(State))
	{
		InputStateRequests.Remove(State);
	}

	// 창끼리 겹치면 우선순위가 높은 창 아래에 끼운다. 높은 창이 닫히면 그때 드러난다.
	const UYSDeveloperSettings* Settings = GetDefault<UYSDeveloperSettings>();
	const FYSInputWindowRule* NewRule = Settings->FindInputWindowRule(State);
	int32 InsertIndex = InputStateRequests.Num();

	if ( NewRule != nullptr )
	{
		for ( int32 Index = 0; Index < InputStateRequests.Num(); ++Index )
		{
			const FYSInputWindowRule* OtherRule = Settings->FindInputWindowRule(InputStateRequests[Index]);

			if ( OtherRule != nullptr && OtherRule->Priority > NewRule->Priority )
			{
				InsertIndex = Index;
				break;
			}
		}
	}

	InputStateRequests.Insert(State, InsertIndex);

	TransitionState(InputStateRequests.Last());
}

void UYSInputStateMachineComponent::RemoveStateStack(FName State)
{
	// 바닥 상태는 빠지지 않는다.
	if ( State != BaseInputState )
	{
		InputStateRequests.Remove(State);
	}
	
	if ( InputStateRequests.IsEmpty() )
		return;
	
	TransitionState(InputStateRequests.Last());
}
