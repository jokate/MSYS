// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/Task/YSAT_Trace.h"

#include "AbilitySystemComponent.h"
#include "Ability/YSGameplayAbility.h"
#include "AttackableActor/YSTraceObject.h"
#include "Library/YSBlueprintFunctionLibrary.h"

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
	
	// 분신이 실행한 어빌리티면 Master 의 것을 쓴다.
	AActor* DamageInstigator = UYSBlueprintFunctionLibrary::ResolveDamageInstigator(AvatarActor);

	const UYSGameplayAbility* OwningAbility = Cast<UYSGameplayAbility>(Ability);
	const float EffectScale = IsValid(OwningAbility) ? OwningAbility->GetEffectScale() : 1.f;
	const TSharedPtr<FYSAbilityHitContext> HitContext = IsValid(OwningAbility) ? OwningAbility->GetHitContext() : TSharedPtr<FYSAbilityHitContext>();

	TraceObject = UYSTraceObject::Create(this, AvatarActor, DamageInstigator, TraceConfig, EffectScale, HitContext);
	TraceObject->OnTraceEnded.AddDynamic(this, &UYSAT_Trace::_OnTraceEnded);
	TraceObject->Start();
}

void UYSAT_Trace::OnDestroy(bool bInOwnerFinished)
{
	if ( IsValid(TraceObject) )
	{
		TraceObject->Stop();
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

void UYSAT_Trace::_OnTraceEnded()
{
	EndTask();
}
