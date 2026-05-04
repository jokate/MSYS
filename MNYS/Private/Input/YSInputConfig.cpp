// Fill out your copyright notice in the Description page of Project Settings.


#include "Input/YSInputConfig.h"

const UInputAction* UYSInputConfig::FindInputActionForTag(const FGameplayTag& InputTag) const
{
	for (const FTaggedInputAction& TaggedInputAction : TaggedInputActions)
    {
    	if (TaggedInputAction.InputAction && TaggedInputAction.InputTag == InputTag)
    	{
    		return TaggedInputAction.InputAction;
    	}
    }

    return nullptr;
}
