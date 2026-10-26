// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/YSCombatProfile.h"

UYSCombatProfile::UYSCombatProfile()
{
	FYSTurnBand Small;
	Small.MaxAngle = 45.f;
	Small.Duration = 0.15f;

	FYSTurnBand Quarter;
	Quarter.MaxAngle = 135.f;
	Quarter.Duration = 1.f;

	FYSTurnBand Half;
	Half.MaxAngle = 180.f;
	Half.Duration = 1.5f;

	TurnBands = { Small, Quarter, Half };

	FYSTurnSourceRule TargetSwitch;
	TurnSourceRules.Add(EYSFaceYawSource::TargetSwitch, TargetSwitch);

	// 어빌리티 자기 이동이다. 자기 태그에 막히거나 자기 몽타주를 끊으면 안 되고, 대시보다 길면 방향이 틀어진다.
	FYSTurnSourceRule AbilityMotion;
	AbilityMotion.bRespectBlockedTags = false;
	AbilityMotion.bAllowTurnMontage = false;
	AbilityMotion.MaxDuration = 0.1f;
	TurnSourceRules.Add(EYSFaceYawSource::AbilityMotion, AbilityMotion);
}

const FYSTurnBand* UYSCombatProfile::FindTurnBand(float AbsDeltaYaw) const
{
	return TurnBands.FindByPredicate([AbsDeltaYaw](const FYSTurnBand& Band)
	{
		return AbsDeltaYaw <= Band.MaxAngle;
	});
}

EYSTargetDistanceBand UYSCombatProfile::RawBand(float Distance) const
{
	if ( Distance <= NearMax )
	{
		return EYSTargetDistanceBand::Near;
	}

	if ( Distance <= MidMax )
	{
		return EYSTargetDistanceBand::Mid;
	}

	if ( Distance <= FarMax )
	{
		return EYSTargetDistanceBand::Far;
	}

	return EYSTargetDistanceBand::OutOfRange;
}

EYSTargetDistanceBand UYSCombatProfile::ResolveDistanceBand(float Distance, EYSTargetDistanceBand Previous) const
{
	const EYSTargetDistanceBand Raw = RawBand(Distance);

	if ( Raw == Previous || BandHysteresis <= 0.f )
	{
		return Raw;
	}

	// 이전 구간 쪽으로 여유만큼 당겨 다시 본다. 여전히 이전 구간이면 아직 넘어가지 않은 것이다.
	const float Biased = ( Raw > Previous ) ? Distance - BandHysteresis : Distance + BandHysteresis;
	return RawBand(Biased) == Previous ? Previous : Raw;
}
