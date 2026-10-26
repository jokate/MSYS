// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/YSSwitchAbusePolicy.h"

#include "GameFramework/Actor.h"

void FYSSwitchAbuseState::Record(const AActor* Target, float Now)
{
	ConsecutiveSameTarget = IsSameAsLastTarget(Target) ? ConsecutiveSameTarget + 1 : 1;
	LastTarget = Target;
	LastOpenTime = Now;

	// 죽은 대상이 쌓이지 않게 기록할 때마다 정리한다.
	for ( TMap<TWeakObjectPtr<const AActor>, float>* Map : { &LastOpenTimeByTarget, &PreviousOpenTimeByTarget } )
	{
		for ( auto It = Map->CreateIterator(); It; ++It )
		{
			if ( It->Key.IsValid() == false )
			{
				It.RemoveCurrent();
			}
		}
	}

	if ( IsValid(Target) )
	{
		PreviousOpenTimeByTarget.Add(Target, GetLastOpenTime(Target));
		LastOpenTimeByTarget.Add(Target, Now);
	}
}

float FYSSwitchAbuseState::GetLastOpenTime(const AActor* Target) const
{
	const float* Found = LastOpenTimeByTarget.Find(Target);
	return Found ? *Found : -BIG_NUMBER;
}

float FYSSwitchAbuseState::GetPreviousOpenTime(const AActor* Target) const
{
	const float* Found = PreviousOpenTimeByTarget.Find(Target);
	return Found ? *Found : -BIG_NUMBER;
}

bool FYSSwitchAbuseState::IsSameAsLastTarget(const AActor* Target) const
{
	return IsValid(Target) && LastTarget.Get() == Target;
}

bool FYSSwitchAbusePolicy_GlobalCooldown::CanOpen(const FYSSwitchAbuseState& State, const AActor* Target, float Now) const
{
	return ( Now - State.LastOpenTime ) >= Cooldown;
}

float FYSSwitchAbusePolicy_GlobalCooldown::GetCooldownRemaining(const FYSSwitchAbuseState& State, const AActor* Target, float Now) const
{
	return FMath::Max(0.f, Cooldown - ( Now - State.LastOpenTime ));
}

float FYSSwitchAbusePolicy_PerTargetDecay::GetEffectScale(const FYSSwitchAbuseState& State, const AActor* Target, float Now) const
{
	// 배율은 창이 열린 뒤 적중 때 읽는다. 마지막 기록은 이번 창 자신이라 그 전 기록과 비교한다.
	return ( Now - State.GetPreviousOpenTime(Target) ) < DecayWindow ? DecayScale : 1.f;
}

float FYSSwitchAbusePolicy_PerTargetDecay::GetCooldownRemaining(const FYSSwitchAbuseState& State, const AActor* Target, float Now) const
{
	// 지금 이 대상에게 창을 열면 감쇠가 걸리는 남은 시간. 기록 없는 대상 · 빈 타겟은 0.
	return FMath::Max(0.f, DecayWindow - ( Now - State.GetLastOpenTime(Target) ));
}

float FYSSwitchAbusePolicy_EscalatingCost::GetGaugeCost(const FYSSwitchAbuseState& State, const AActor* Target, float BaseCost) const
{
	const int32 Streak = State.IsSameAsLastTarget(Target) ? State.ConsecutiveSameTarget : 0;
	return BaseCost + CostStep * Streak;
}
