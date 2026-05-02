// Fill out your copyright notice in the Description page of Project Settings.


#include "Input/StateMachine/YSInputStates.h"

#include "YSAbilitySystemComponent.h"

void UYSInputStates::ProcessInput(const FGameplayTag& InputGameplayTag)
{
	UYSAbilitySystemComponent* ASC = OwnerASC.Get();

	// Input에 대한 Consume
	if ( IsValid(ASC) == false )
	{
		return;
	}

	// 여기에 들어온 input의 경우에는 다음과 같이 설정됨.
	// StateType.InputType.Pressed / Released.
	FString StateTagStr = FString::Printf(TEXT("%s.%s"), *StateName, *InputGameplayTag.ToString());

	// GameplayTag로 변환
	FGameplayTag FinalizedTag =  FGameplayTag::RequestGameplayTag(FName(*StateTagStr));
	ASC->ProcessAbilityByInputPass(FinalizedTag);
}
