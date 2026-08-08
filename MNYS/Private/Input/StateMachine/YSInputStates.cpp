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
	
	const FGameplayTag FinalizedTag = ResolveStateTag(InputGameplayTag);
	
	if ( FinalizedTag.IsValid() == false )
	{
		return;
	}

	// 여기에 인풋 히스토리를 넣어줘야 할 거 같음.
	// -> 콤보 처리 및 관리를 위해서이긴 한데, 만약에 한번에 여러개가 들어온다는 가정하에서는 조금 꼬롬함.
	// 그리고 사실상 반복입력의 우려가 있음 -> 차라리 존재하면 시간에 대한 업데이트 처리만 하면 될 거 같음.
	
	//In
	InputStateMachineComponent->RecordInputHistory(FinalizedTag, InputPhase);
	
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
	
	if (UYSInputStateMachineComponent* InputStateMachineComponent = UYSInputStateMachineComponent::Get(Owner))
	{
		OwnerStateMachineManager = InputStateMachineComponent;
	}
}
