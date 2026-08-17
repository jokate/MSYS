// Fill out your copyright notice in the Description page of Project Settings.


#include "Input/Combo/YSComboData.h"

bool FYSCommandSequence::IsSatisfiedCommand(const TArray<FYSTagHistory>& InputGameplayTags) const
{
	for (int32 i = 0; i < CommandSequence.Num(); ++i)
	{
		if ( InputGameplayTags.IsValidIndex(i) == false )
			return false;

		if ( InputGameplayTags[i].InputTag.MatchesTagExact(CommandSequence[i]) == false )
			return false;
	}

	return true;
}
