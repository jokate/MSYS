// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/YSGameModeBase.h"

#include "GenericTeamAgentInterface.h"

namespace
{
	// CustomTimeDilation 이 0 이면 이동 계산에서 0 으로 나누는 경로가 생긴다. 멈춤은 이 값으로 대신한다.
	constexpr float MinTimeDilation = 0.0001f;
}

bool FYSTimeDilationFactor::AppliesTo(const AActor* Actor) const
{
	if ( Scope == EYSTimeDilationScope::All )
	{
		return true;
	}

	const AActor* Owner = ScopeOwner.Get();

	if ( Scope == EYSTimeDilationScope::Participants )
	{
		return Actor == Owner || ScopeActors.Contains(Actor);
	}

	if ( IsValid(Owner) == false || Actor == Owner )
	{
		return false;
	}

	const IGenericTeamAgentInterface* TeamAgent = Cast<const IGenericTeamAgentInterface>(Owner);

	if ( TeamAgent == nullptr )
	{
		return false;
	}

	return TeamAgent->GetTeamAttitudeTowards(*Actor) != ETeamAttitude::Friendly;
}

void AYSGameModeBase::SetTimeDilation()
{
	for (TWeakObjectPtr<AActor> TimeDilationTarget : TimeDilationActors )
	{
		AActor* TimeDilationActor = TimeDilationTarget.Get();

		if ( IsValid(TimeDilationActor) == false )
		{
			continue;
		}

		// 액터마다 자기에게 해당하는 가장 최근 요청을 쓴다.
		float Dilation = 1.f;

		for ( int32 Index = TimeDilationFactors.Num() - 1; Index >= 0; --Index )
		{
			if ( TimeDilationFactors[Index].AppliesTo(TimeDilationActor) )
			{
				Dilation = TimeDilationFactors[Index].TimeDilation;
				break;
			}
		}

		TimeDilationActor->CustomTimeDilation = FMath::Max(Dilation, MinTimeDilation);
	}
}

void AYSGameModeBase::RegisterTimeDilation(UObject* Object, float TimeDilation, EYSTimeDilationScope Scope, AActor* ScopeOwner, const TArray<AActor*>& ScopeActors)
{
	if ( Object == nullptr )
		return;

	FYSTimeDilationFactor NewFactor;
	NewFactor.DilationCaller = Object;
	NewFactor.TimeDilation = TimeDilation;
	NewFactor.Scope = Scope;
	NewFactor.ScopeOwner = ScopeOwner;

	for ( AActor* ScopeActor : ScopeActors )
	{
		NewFactor.ScopeActors.Add(ScopeActor);
	}

	TimeDilationFactors.Add(NewFactor);

	SetTimeDilation();
}

void AYSGameModeBase::RemoveTimeDilation(UObject* Object)
{
	if ( Object == nullptr )
		return;

	TimeDilationFactors.RemoveAll([Object](const FYSTimeDilationFactor& Factor) { return Factor == Object; });

	SetTimeDilation();
}

void AYSGameModeBase::StartTimedDilation(float TimeDilation, float Duration, EYSTimeDilationScope Scope, AActor* ScopeOwner, const TArray<AActor*>& ScopeActors)
{
	UWorld* World = GetWorld();

	if ( Duration <= 0.f || IsValid(World) == false )
	{
		return;
	}

	// 요청마다 키가 달라야 한 요청의 해제가 다른 요청을 걷어내지 않는다. 팩터가 강참조로 붙들어 해제 전에는 사라지지 않는다.
	UObject* Caller = NewObject<UObject>(this);
	RegisterTimeDilation(Caller, TimeDilation, Scope, ScopeOwner, ScopeActors);

	// 월드 타이머는 액터 시간 배율을 받지 않아 정지 중에도 제때 풀린다.
	FTimerHandle Handle;
	World->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateWeakLambda(this, [this, WeakCaller = TWeakObjectPtr<UObject>(Caller)]()
	{
		RemoveTimeDilation(WeakCaller.Get());
	}), Duration, false);
}

void AYSGameModeBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	SetTimeDilation();
}
