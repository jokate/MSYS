// Fill out your copyright notice in the Description page of Project Settings.


#include "Input/Combo/YSComboData.h"

// 버퍼의 머리가 아니라 꼬리에서 맞춘다.
// 머리 기준이면 콤보 1타의 Input.Attack 이 버퍼 0번을 점유한 채로 남아,
// 공격 중에 들어온 커맨드는 첫 비교에서 무조건 어긋난다.
bool FYSCommandSequence::IsSatisfiedCommand(const TArray<FYSTagHistory>& InputGameplayTags) const
{
	if ( CommandSequence.Num() == 0 )
	{
		return false;
	}

	const int32 Offset = InputGameplayTags.Num() - CommandSequence.Num();

	if ( Offset < 0 )
	{
		return false;
	}

	for ( int32 i = 0; i < CommandSequence.Num(); ++i )
	{
		if ( InputGameplayTags[Offset + i].InputTag.MatchesTagExact(CommandSequence[i]) == false )
		{
			return false;
		}
	}

	return true;
}
