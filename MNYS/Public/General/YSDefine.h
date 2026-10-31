// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"

// S5 계측 전용 로그. 측정이 끝나면 이 선언과 UE_LOG(LogYSMeasure ...) 줄을 지운다.
DECLARE_LOG_CATEGORY_EXTERN(LogYSMeasure, Log, All);


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
