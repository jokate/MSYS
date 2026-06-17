// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/YSGameModeBase.h"

void AYSGameModeBase::SetTimeDilation()
{
	float TimeDilationTop = 1.f;
	
	if ( TimeDilationFactors.Num() > 0 )
	{
		TimeDilationTop = TimeDilationFactors.Top().TimeDilation;	
	}
	
	for (TWeakObjectPtr<AActor> TimeDilationTarget : TimeDilationActors )
	{
		AActor* TimeDilationActor = TimeDilationTarget.Get();
		
		if ( IsValid(TimeDilationActor) == false )
		{
			continue;
		}
		
		TimeDilationActor->CustomTimeDilation = TimeDilationTop;
	}
}

void AYSGameModeBase::RegisterTimeDilation(UObject* Object, float TimeDilation)
{
	if ( Object == nullptr )
		return;
	
	FYSTimeDilationFactor NewFactor;
	NewFactor.DilationCaller = Object;
	NewFactor.TimeDilation = TimeDilation;
	
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

void AYSGameModeBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	SetTimeDilation();
}
