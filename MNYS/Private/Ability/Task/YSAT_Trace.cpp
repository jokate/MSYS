// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/Task/YSAT_Trace.h"

#include "AbilitySystemComponent.h"
#include "AttackableActor/YSTraceObject.h"

UYSAT_Trace::UYSAT_Trace()
{
	bTickingTask = true;
}

UYSAT_Trace* UYSAT_Trace::CreateTask(UGameplayAbility* InAbility, const FYSTraceConfig& InConfig)
{
	UYSAT_Trace* Task   = NewAbilityTask<UYSAT_Trace>(InAbility);
	Task->TraceConfig = InConfig;
	return Task;
}

void UYSAT_Trace::Activate()
{
	Super::Activate();

	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (!IsValid(ASC))
	{
		EndTask();
		return;
	}

	AActor* AvatarActor = ASC->GetAvatarActor();
	if (!IsValid(AvatarActor))
	{
		EndTask();
		return;
	}

	TraceObject = UYSTraceObject::Create(this, AvatarActor, AvatarActor, TraceConfig);
	TraceObject->OnTraceHit.AddDynamic(this, &UYSAT_Trace::_OnTraceObjectHit);
	TraceObject->OnHitCountDepleted.AddDynamic(this, &UYSAT_Trace::_OnHitCountDepleted);

	if (TraceConfig.bTraceOnce)
	{
		TraceObject->ExecuteOnce();
		EndTask();
	}
}

void UYSAT_Trace::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);

	if (IsValid(TraceObject))
	{
		TraceObject->Tick(DeltaTime);
	}
}

void UYSAT_Trace::_OnTraceObjectHit(const TArray<FHitResult>& HitResults, const FName& DamageRow)
{
	OnTraceHit.Broadcast(HitResults, DamageRow);
}

void UYSAT_Trace::_OnHitCountDepleted()
{
	EndTask();
}
