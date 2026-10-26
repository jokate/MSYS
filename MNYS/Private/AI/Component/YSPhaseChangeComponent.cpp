// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Component/YSPhaseChangeComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AI/Component/YSPhaseChangeCondition.h"
#include "Character/YSCharacterBase.h"
#include "General/YSGameplayTag.h"

UYSPhaseChangeComponent::UYSPhaseChangeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	DeferTags = { YSTags::State_Groggy_Broken };
}

void UYSPhaseChangeComponent::BeginPlay()
{
	Super::BeginPlay();
	
	OwnerCharacter = GetOwner<AYSCharacterBase>();
}

void UYSPhaseChangeComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                            FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	CheckPhaseChange();
}

void UYSPhaseChangeComponent::CheckPhaseChange()
{
	if ( OwnerCharacter.IsValid() == false )
	{
		return; 
	}
	
	int32 Index = CurrentPhase - 1;
	AYSCharacterBase* CharacterBase = OwnerCharacter.Get();
	if ( PhaseChangeConditions.IsValidIndex(Index) == false )
	{
		return;
	}
	
	const FYSPhaseChangeConditions& PhaseChangeCondition = PhaseChangeConditions[Index];
	
	if ( PhaseChangeCondition.CheckCondition(CharacterBase))
	{
		if ( IsDeferred(CharacterBase) )
		{
			return;
		}

		// StateTree 쪽에다가 이벤트 송신.

		++CurrentPhase;
		OnPhaseChanged.Broadcast(CurrentPhase);
	}
}

bool UYSPhaseChangeComponent::IsDeferred(AActor* Owner) const
{
	const UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);

	if ( IsValid(ASC) == false )
	{
		return false;
	}

	for ( const FGameplayTag& Tag : DeferTags )
	{
		if ( ASC->HasMatchingGameplayTag(Tag) )
		{
			return true;
		}
	}

	return false;
}

