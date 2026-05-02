// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/Task/YSAT_Trace.h"

#include "AbilitySystemComponent.h"
#include "Ability/TargetActor/YSTA_Trace.h"

UYSAT_Trace::UYSAT_Trace(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	TargetActor = nullptr;
}

UYSAT_Trace* UYSAT_Trace::CreateTask(UGameplayAbility* OwningAbility, const FName& TargetDamageInfo,
	const AActor* DamageCauser)
{
	UYSAT_Trace* NewTask = NewAbilityTask<UYSAT_Trace>(OwningAbility);
    NewTask->TargetDamageInfo = TargetDamageInfo;
	NewTask->TargetActor = DamageCauser;
    return NewTask;
}

void UYSAT_Trace::Activate()
{
	Super::Activate();
	SetupTraceData();
	SetWaitingOnAvatar();
}

void UYSAT_Trace::OnDestroy(bool bInOwnerFinished)
{
	Super::OnDestroy(bInOwnerFinished);
}

void UYSAT_Trace::SetupTraceData()
{
	AActor* AvatarActor = GetAvatarActor();

	// 차후 추가 개편.
	/*FMUDamageInfo DamageInfo;
	if ( UMUFunctionLibrary::GetDamageInfo(this, TargetDamageInfo, DamageInfo) == false  || IsValid(AvatarActor) == false )
	{
		OnComplete.Broadcast(FGameplayAbilityTargetDataHandle());
		return;
	}

	for ( const FMUDamageRange& DamageRange : DamageInfo.DamageRanges )
	{
		TSubclassOf<AMUTA_Trace> TraceClass = DamageRange.TraceClass;
		const FInstancedStruct& RangeConfig = DamageRange.RangeConfig;

		AMUTA_Trace* Trace = SpawnAndInitializeTargetActor(TraceClass);

		if ( IsValid(Trace) == false )
		{
			continue;
		}
		FTransform FinalTransform = AvatarActor->GetActorTransform();
		FName AttachmentSocket = NAME_None;
		if (AMUTA_BoxTrace* BoxTrace = Cast<AMUTA_BoxTrace>(Trace))
		{
			const FMUTraceRangeConfig_Box* Cfg = RangeConfig.GetPtr<FMUTraceRangeConfig_Box>();
			if ( Cfg != nullptr )
			{
				FinalTransform += Cfg->OffSetTransform;
				AttachmentSocket = Cfg->AttachmentSocketName;
				BoxTrace->SetupBoxExtent(Cfg->BoxExtent);
			}
		}

		if ( AMUTA_CapsuleTrace* CapsuleTrace = Cast<AMUTA_CapsuleTrace>(Trace))
		{
			const FMUTraceRangeConfig_Capsule* Cfg = RangeConfig.GetPtr<FMUTraceRangeConfig_Capsule>();

			if ( Cfg != nullptr )
			{
				FinalTransform += Cfg->OffSetTransform;
				AttachmentSocket = Cfg->AttachmentSocketName;
				CapsuleTrace->SetupCapsule(Cfg->HalfHeight, Cfg->Radius);
			}
		}

		if ( AMUTA_SphereTrace* SphereTrace = Cast<AMUTA_SphereTrace>(Trace))
		{
			const FMUTraceRangeConfig_Sphere* Cfg = RangeConfig.GetPtr<FMUTraceRangeConfig_Sphere>();

			if ( Cfg != nullptr )
			{
				FinalTransform += Cfg->OffSetTransform;
				AttachmentSocket = Cfg->AttachmentSocketName;
				SphereTrace->SetupRadius(Cfg->Radius);
			}
		}

		FinalizeTargetActor(Trace, FinalTransform, AttachmentSocket);
	}*/
}

AYSTA_Trace* UYSAT_Trace::SpawnAndInitializeTargetActor(TSubclassOf<AYSTA_Trace> TraceClass)
{
	AYSTA_Trace* SpawnedTargetActor = Ability->GetWorld()->SpawnActorDeferred<AYSTA_Trace>(TraceClass, FTransform::Identity, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	if (SpawnedTargetActor)
	{
		SpawnedTargetActor->TargetDataReadyDelegate.AddUObject(this, &UYSAT_Trace::OnTargetDataReadyCallback);
		SpawnedTargetActor->InitializeData(TargetDamageInfo);
	}

	return SpawnedTargetActor;
}

void UYSAT_Trace::FinalizeTargetActor(AYSTA_Trace* TraceTarget, const FTransform& FinalizeTransform, const FName& AttachmentSocket)
{
	if ( IsValid(TargetActor) == false )
		return;
	
	USkeletalMeshComponent* SkeletalComponent = TargetActor->GetComponentByClass<USkeletalMeshComponent>();
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();

	if ( IsValid(SkeletalComponent) == false || IsValid(ASC) == false )
	{
		return;
	}
	
	TraceTarget->FinishSpawning(FinalizeTransform);
	ASC->SpawnedTargetActors.Push(TraceTarget);
	TraceTarget->StartTargeting(Ability);
	TraceTarget->AttachToComponent(SkeletalComponent,
		AttachmentSocket == NAME_None ?
		FAttachmentTransformRules::KeepWorldTransform
		: FAttachmentTransformRules::SnapToTargetIncludingScale
		, AttachmentSocket);
	
	SpawnedTargetActors.AddUnique(TraceTarget);
}

void UYSAT_Trace::OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& DataHandle)
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnComplete.Broadcast(DataHandle);
	}

	EndTask();
}
