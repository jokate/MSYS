// Fill out your copyright notice in the Description page of Project Settings.


#include "Input/StateMachine/YSInputStates.h"

#include "AbilitySystemInterface.h"
#include "YSAbilitySystemComponent.h"
#include "General/YSGeneratedGameplayTags.h"
#include "Input/StateMachine/YSInputStateMachineComponent.h"

void UYSInputStates::ProcessInput(const FGameplayTag& InputGameplayTag, EYSInputPhase InputPhase)
{
	UYSAbilitySystemComponent* ASC = OwnerASC.Get();
	UYSInputStateMachineComponent* InputStateMachineComponent = OwnerStateMachineManager.Get();
	// Input에 대한 Consume
	if ( IsValid(ASC) == false || IsValid(InputStateMachineComponent) == false )
	{
		return;
	}
	
	TArray<FGameplayTag> Candidates;
	ResolveStateTags(InputGameplayTag, Candidates);

	if ( Candidates.Num() == 0 )
	{
		return;
	}

	// 히스토리는 세부 태그로만 남긴다.
	// FYSPlaybackCondition_Input 이 정확 비교라, 후보를 다 넣으면 어느 것을 조건에 써야 할지 모호해진다.
	InputStateMachineComponent->RecordInputHistory(Candidates[0], InputPhase);

	ASC->ProcessAbilityByInputPass(Candidates, InputPhase);
}

void UYSInputStates::ResolveStateTags(const FGameplayTag& InputGameplayTag, TArray<FGameplayTag>& OutTags)
{
	if ( const TArray<FGameplayTag>* Cached = ResolvedTagCache.Find(InputGameplayTag) )
	{
		OutTags = *Cached;
		return;
	}

	TArray<FString> Scopes;
	Scopes.Add(StateName);
	Scopes.Append(CategoryNames);

	TArray<FGameplayTag> Resolved;

	for ( const FString& Scope : Scopes )
	{
		const FString TagStr = FString::Printf(TEXT("%s.%s"), *Scope, *InputGameplayTag.ToString());
		const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(*TagStr), /*ErrorIfNotFound=*/false);

		if ( Tag.IsValid() )
		{
			Resolved.Add(Tag);
		}
	}

	ResolvedTagCache.Emplace(InputGameplayTag, Resolved);
	OutTags = Resolved;
}

void UYSInputStates::InitState(AActor* Owner)
{
	OwnerActor = Owner;
	if ( IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Owner) )
	{
		OwnerASC = Cast<UYSAbilitySystemComponent>(ASI->GetAbilitySystemComponent());
	}
	
	if (UYSInputStateMachineComponent* InputStateMachineComponent = UYSInputStateMachineComponent::Get(Owner))
	{
		OwnerStateMachineManager = InputStateMachineComponent;
	}
}
