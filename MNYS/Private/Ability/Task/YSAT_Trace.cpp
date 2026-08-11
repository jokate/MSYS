// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/Task/YSAT_Trace.h"

#include "AbilitySystemComponent.h"
#include "Ability/YSGameplayAbility.h"
#include "Ability/AbilityComponent/YSAbilityPlayback.h"
#include "AttackableActor/YSTraceObject.h"
#include "Interface/YSDamageProxy.h"

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
	
	// ASC 자체에서 가져오되 만약 분신이 실행한 어빌이라면 Master껄 가지고 오자..
	AActor* DamageInstigator = AvatarActor;
	if ( const IYSDamageProxy* Proxy = Cast<IYSDamageProxy>(AvatarActor) )
	{
		DamageInstigator = Proxy->GetDamageInstigator();
	}

	TraceObject = UYSTraceObject::Create(this, AvatarActor, DamageInstigator, TraceConfig);
	TraceObject->OnTraceHit.AddDynamic(this, &UYSAT_Trace::_OnTraceObjectHit);
	TraceObject->OnHitCountDepleted.AddDynamic(this, &UYSAT_Trace::_OnHitCountDepleted);

	if (TraceConfig.bTraceOnce)
	{
		TraceObject->ExecuteOnce();
		EndTask();
	}
}

void UYSAT_Trace::OnDestroy(bool bInOwnerFinished)
{
	UYSGameplayAbility* OwningAbility = Cast<UYSGameplayAbility>(Ability);
	
	if ( IsValid(OwningAbility))
	{
		FYSAbilityHitContext* HitContext = OwningAbility->GetHitContext().Get();
		if ( HitContext != nullptr )
		{
			HitContext->RemoveTraceEntry(TraceObject);
		}
	} 
	
	TraceObject = nullptr;
	Super::OnDestroy(bInOwnerFinished);
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
	UYSGameplayAbility* OwningAbility = Cast<UYSGameplayAbility>(Ability);
	
	if ( IsValid(OwningAbility) == false )
	{
		return;
	}
	
	FYSAbilityHitContext* HitContext = OwningAbility->GetHitContext().Get();
	
	if ( HitContext != nullptr )
	{
		for ( const FHitResult& HitResult : HitResults)
		{
			HitContext->AddHitActor(HitResult.GetActor());
			HitContext->UpdateHitResult(TraceObject, HitResult);
		}
	}
	
	UYSAbilityPlaybackBase* AbilityPlaybackBase = OwningAbility->GetCurrentPlayback();
		
	if ( IsValid(AbilityPlaybackBase) )
	{
		AbilityPlaybackBase->OnHit(HitResults);
	}
}

void UYSAT_Trace::_OnHitCountDepleted()
{
	EndTask();
}
