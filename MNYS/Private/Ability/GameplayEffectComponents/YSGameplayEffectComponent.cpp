// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/GameplayEffectComponents/YSGameplayEffectComponent.h"

#include "GameplayEffect.h"

bool UYSGameplayEffectComponent::OnActiveGameplayEffectAdded(FActiveGameplayEffectsContainer& ActiveGEContainer,
                                                             FActiveGameplayEffect& ActiveGE) const
{
	if ( ActiveGEContainer.IsNetAuthority() )
	{
		ActiveGE.EventSet.OnEffectRemoved.AddUObject(this, &UYSGameplayEffectComponent::OnActiveGameplayEffectRemoved, &ActiveGEContainer);
	}
	
	return true;
}
