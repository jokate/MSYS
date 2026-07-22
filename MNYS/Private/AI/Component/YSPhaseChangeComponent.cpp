// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Component/YSPhaseChangeComponent.h"

#include "AI/Component/YSPhaseChangeCondition.h"

UYSPhaseChangeComponent::UYSPhaseChangeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
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
		// StateTree 쪽에다가 이벤트 송신.
		
		++CurrentPhase;
	}
}

