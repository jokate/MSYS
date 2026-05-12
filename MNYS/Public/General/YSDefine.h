// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"


#define YS_PLAYER 255
#define YS_MONSTER_END 254
#define YS_MONSTER_START 128
#define YS_NEUTRAL_END 127

namespace YSTeamFunction
{
	inline ETeamAttitude::Type GeneralTeamSolver(FGenericTeamId A, FGenericTeamId B)
	{
		uint8 AId = A.GetId();
		uint8 BId = B.GetId();

		bool bANeutral = AId <= YS_NEUTRAL_END;
		bool bBNeutral = BId <= YS_NEUTRAL_END;
		if ( AId == BId )
		{
			return ETeamAttitude::Friendly;
		}
		
		if ( bANeutral || bBNeutral )
			return ETeamAttitude::Neutral;

		return ETeamAttitude::Hostile;
	}
}
