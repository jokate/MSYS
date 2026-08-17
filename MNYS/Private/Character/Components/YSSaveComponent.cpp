// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Components/YSSaveComponent.h"

#include "YSAbilitySystemComponent.h"
#include "Ability/YSGameplayAbility.h"
#include "General/YSGameplayTag.h"
#include "General/YSInputGameplayTags.h"
#include "Input/StateMachine/YSInputStateMachineComponent.h"


bool FYSSavedTechnique::IsValid() const
{
	return AbilityClass != nullptr && PlaybackIndex != INDEX_NONE;
}

void FYSSavedTechnique::Reset()
{
	AbilityClass = nullptr;
	PlaybackIndex = INDEX_NONE;
	SavedMultiplier = 1.f;
}


UYSSaveComponent* UYSSaveComponent::Get(AActor* Owner)
{
	if ( IsValid(Owner) == false )
	{
		return nullptr;
	}

	return Owner->FindComponentByClass<UYSSaveComponent>();
}

UYSSaveComponent::UYSSaveComponent()
{
	// 슬롯은 사건이 있을 때만 바뀐다. 매 프레임 볼 것이 없다.
	PrimaryComponentTick.bCanEverTick = false;
}

void UYSSaveComponent::BeginPlay()
{
	Super::BeginPlay();

	SavedTechniques.Reserve(MaxSlotCount);
	RefreshStateTags();
	
	if ( UYSInputStateMachineComponent* InputStateMachine = UYSInputStateMachineComponent::Get(GetOwner()) )
	{
		InputStateMachine->OnRawInputAccepted.AddUniqueDynamic(this, &UYSSaveComponent::HandleRawInput);
	}

	RefreshStateTags();
}

void UYSSaveComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if ( UYSInputStateMachineComponent* InputStateMachine = UYSInputStateMachineComponent::Get(GetOwner()) )
	{
		InputStateMachine->OnRawInputAccepted.RemoveDynamic(this, &UYSSaveComponent::HandleRawInput);
	}
	Super::EndPlay(EndPlayReason);
}

void UYSSaveComponent::MarkSavable(TSubclassOf<UYSGameplayAbility> AbilityClass, int32 PlaybackIndex, float Multiplier)
{
	if ( AbilityClass == nullptr || PlaybackIndex == INDEX_NONE )
	{
		return;
	}

	UWorld* World = GetWorld();

	if ( IsValid(World) == false )
	{
		return;
	}

	// 최신 거로 덮어쓰긴 할 거임.
	PendingTechnique = FYSSavedTechnique(AbilityClass, PlaybackIndex, Multiplier);
	RefreshStateTags();
	OnSaveStateChanged.Broadcast();
}

bool UYSSaveComponent::TryCommitPending()
{
	if ( PendingTechnique.IsValid() == false )
	{
		return false;
	}

	// 찼으면 저장 데이터를 소진하지 않는 이상 절대 소모 불가.
	if ( IsSlotFull() )
	{
		return false;
	}

	SavedTechniques.Add(PendingTechnique);
	RefreshStateTags();
	OnSaveStateChanged.Broadcast();

	return true;
}

bool UYSSaveComponent::TryConsumeSlot(FYSSavedTechnique& OutTechnique)
{
	if ( SavedTechniques.Num() <= 0 )
	{
		return false;
	}
	
	OutTechnique = SavedTechniques[0];
	SavedTechniques.RemoveAt(0);

	RefreshStateTags();
	OnSaveStateChanged.Broadcast();

	return true;
}

void UYSSaveComponent::SetMaxSlotCount(int32 NewMax)
{
	MaxSlotCount = FMath::Max(1, NewMax);
	
	while ( SavedTechniques.Num() > MaxSlotCount )
	{
		SavedTechniques.RemoveAt(0);
	}

	RefreshStateTags();
	OnSaveStateChanged.Broadcast();
}

void UYSSaveComponent::HandleRawInput(const FGameplayTag& InputTag, EYSInputPhase InputPhase)
{
	if ( InputTag != YSInputTags::InputSave || InputPhase != EYSInputPhase::Pressed )
	{
		return;
	}

	TryCommitPending();
}

void UYSSaveComponent::ClearPending()
{
	if ( PendingTechnique.IsValid() == false )
	{
		return;
	}

	PendingTechnique.Reset();

	RefreshStateTags();
	OnSaveStateChanged.Broadcast();
}

void UYSSaveComponent::RefreshStateTags()
{
	UYSAbilitySystemComponent* ASC = GetOwnerASC();

	if ( IsValid(ASC) == false )
	{
		return;
	}
	
	ASC->SetLooseGameplayTagCount(YSTags::State_Save_HasSlot, HasSavedTechnique() ? 1 : 0);
}

UYSAbilitySystemComponent* UYSSaveComponent::GetOwnerASC() const
{
	return UYSAbilitySystemComponent::Get(GetOwner());
}