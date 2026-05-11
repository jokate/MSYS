// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/Task/YSAT_Trace.h"

#include "AbilitySystemComponent.h"

UYSAT_Trace* UYSAT_Trace::CreateTask(UGameplayAbility* InAbility, const FYSTraceConfig& InConfig)
{
	UYSAT_Trace* Task = NewAbilityTask<UYSAT_Trace>(InAbility);
	Task->TraceConfig = InConfig;

	return Task;
}

void UYSAT_Trace::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);

	_TraceByConfig();
}

void UYSAT_Trace::Activate()
{
	Super::Activate();

	bHasPreviousLocation = false;

	if (TraceConfig.bTraceOnce)
	{
		_TraceByConfig();
		EndTask();
	}
}

void UYSAT_Trace::_TraceByConfig()
{
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (!IsValid(ASC))
		return;

	AActor* AvatarActor = ASC->GetAvatarActor();
	if (!IsValid(AvatarActor))
		return;

	USkeletalMeshComponent* Mesh = AvatarActor->FindComponentByClass<USkeletalMeshComponent>();
	if (!IsValid(Mesh))
		return;

	UWorld* World = GetWorld();
	if (!IsValid(World))
		return;

	FVector Location = AvatarActor->GetActorLocation();
	FTransform Transform = AvatarActor->GetActorTransform();
	if ( TraceConfig.SocketName != NAME_None)
	{
		Transform = Mesh->GetSocketTransform(TraceConfig.SocketName);
		Location = Transform.GetLocation();
	}
	else if ( TraceConfig.RelativeLocation != FVector::ZeroVector)
	{
		Location = Transform.TransformPosition(TraceConfig.RelativeLocation);
	}
	
	const FVector Start = bHasPreviousLocation ? PreviousSocketLocation : Location;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(AvatarActor);

	TArray<FHitResult> RawHits;
	bool bHit = false;

	switch (TraceConfig.Shape)
	{
	case EYSTraceShape::Box:
		bHit = World->SweepMultiByChannel(RawHits, Start, Location, Transform.GetRotation(),
			TraceConfig.Channel.GetValue(),
			FCollisionShape::MakeBox(TraceConfig.Extent), Params);
		break;

	case EYSTraceShape::Sphere:
		bHit = World->SweepMultiByChannel(
			RawHits, Start, Location, FQuat::Identity,
			TraceConfig.Channel.GetValue(),
			FCollisionShape::MakeSphere(TraceConfig.Extent.X), Params);
		break;

	case EYSTraceShape::Capsule:
		bHit = World->SweepMultiByChannel(
			RawHits, Start, Location, Transform.GetRotation(),
			TraceConfig.Channel.GetValue(),
			FCollisionShape::MakeCapsule(TraceConfig.Extent.X, TraceConfig.Extent.Y), Params);
		break;

	case EYSTraceShape::Line:
		{
			const FVector LineEnd = Location + Transform.GetUnitAxis(EAxis::X) * TraceConfig.Extent.X;
			bHit = World->LineTraceMultiByChannel(
				RawHits, Location, LineEnd,
				TraceConfig.Channel.GetValue(), Params);
		}
		break;
	}

	PreviousSocketLocation = Location;
	bHasPreviousLocation = true;

	if (!bHit) return;

	const float CurrentTime = World->GetTimeSeconds();
	TArray<FHitResult> ValidHits;

	for (const FHitResult& Hit : RawHits)
	{
		AActor* HitActor = Hit.GetActor();
		if (!IsValid(HitActor)) continue;

		if (TraceConfig.bContinuousHit)
		{
			if (const float* LastHitTime = HitTimeMap.Find(HitActor))
			{
				if (CurrentTime - *LastHitTime < TraceConfig.HitTimeCheck) continue;
			}
			HitTimeMap.Add(HitActor, CurrentTime);
		}
		else
		{
			if (TracedActors.Contains(HitActor)) continue;
			TracedActors.Add(HitActor);
		}

		ValidHits.Add(Hit);
	}

	if (ValidHits.Num() > 0)
	{
		OnTraceHit.Broadcast(ValidHits);
	}
}
