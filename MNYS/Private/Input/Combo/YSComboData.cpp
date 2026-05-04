// Fill out your copyright notice in the Description page of Project Settings.


#include "Input/Combo/YSComboData.h"

bool FYSComboSequence::IsSatisfiedCombo(const TArray<FGameplayTag>& InputGameplayTags) const
{
	for (int32 i = 0; i < ComboSequence.Num(); ++i)
	{
		if ( InputGameplayTags.IsValidIndex(i) == false )
			return false;

		if ( InputGameplayTags[i].MatchesTagExact(ComboSequence[i]) == false )
			return false;
	}

	return true;
}
