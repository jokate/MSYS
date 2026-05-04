// Fill out your copyright notice in the Description page of Project Settings.


#include "Input/YSEnhancedInputComponent.h"


// Sets default values for this component's properties
UYSEnhancedInputComponent::UYSEnhancedInputComponent()
{
}

const FGameplayTag& UYSEnhancedInputComponent::GetGameplayTagByInputAction(const UInputAction* InputAction)
{
	const FInputActionWrapper InputActionWrapper(InputAction);

	if (!InputGameplayTagMap.Contains(InputActionWrapper))
	{
		return FGameplayTag::EmptyTag;
	}

	return InputGameplayTagMap[InputActionWrapper]; 
}

void UYSEnhancedInputComponent::ClearActionBindings()
{
	Super::ClearActionBindings();
	InputGameplayTagMap.Empty();
}
