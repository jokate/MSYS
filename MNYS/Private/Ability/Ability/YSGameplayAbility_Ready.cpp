// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/Ability/YSGameplayAbility_Ready.h"

bool UYSGameplayAbility_Ready::TryTransition(const FGameplayTag& InputGameplayTag, EYSInputPhase InputPhase)
{
	
	if ( ReadyCancelInfo == FYSInputPhase{InputGameplayTag, InputPhase} )
	{
		// Handle cancel logic here
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return true;
	}

	return Super::TryTransition(InputGameplayTag, InputPhase);
}
