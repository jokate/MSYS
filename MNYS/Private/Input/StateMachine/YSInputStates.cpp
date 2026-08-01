// Fill out your copyright notice in the Description page of Project Settings.


#include "Input/StateMachine/YSInputStates.h"

#include "AbilitySystemInterface.h"
#include "YSAbilitySystemComponent.h"
#include "General/YSGeneratedGameplayTags.h"

void UYSInputStates::ProcessInput(const FGameplayTag& InputGameplayTag, EYSInputPhase InputPhase)
{
	UYSAbilitySystemComponent* ASC = OwnerASC.Get();

	// Input에 대한 Consume
	if ( IsValid(ASC) == false )
	{
		return;
	}
	
	const FGameplayTag FinalizedTag = ResolveStateTag(InputGameplayTag);
	
	if ( FinalizedTag.IsValid() == false )
	{
		return;
	}

	ASC->ProcessAbilityByInputPass(FinalizedTag, InputPhase);
}

FGameplayTag UYSInputStates::ResolveStateTag(const FGameplayTag& InputGameplayTag)
{
	if ( const FGameplayTag* Cached = ResolvedTagCache.Find(InputGameplayTag) )
	{
		return *Cached;
	}

	const FString StateTagStr = FString::Printf(TEXT("%s.%s"), *StateName, *InputGameplayTag.ToString());
	
	const FGameplayTag Resolved = FGameplayTag::RequestGameplayTag(FName(*StateTagStr), /*ErrorIfNotFound=*/false);

	ResolvedTagCache.Emplace(InputGameplayTag, Resolved);
	return Resolved;
}

void UYSInputStates::InitState(AActor* Owner)
{
	OwnerActor = Owner;
	if ( IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Owner) )
	{
		OwnerASC = Cast<UYSAbilitySystemComponent>(ASI->GetAbilitySystemComponent());
	}
}
