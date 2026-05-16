// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/Task/YSAT_Trace.h"

#include "AbilitySystemComponent.h"
#include "DrawDebugHelpers.h"

UYSAT_Trace::UYSAT_Trace()
{
	bTickingTask = true;
}

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
	FVector LineEnd = Location;

	switch (TraceConfig.Shape)
	{
	case EYSTraceShape::Box:
		bHit = World->SweepMultiByProfile(RawHits, Start, Location, Transform.GetRotation(),
			TraceConfig.CollisionProfile,
			FCollisionShape::MakeBox(TraceConfig.Extent), Params);
		break;

	case EYSTraceShape::Sphere:
		bHit = World->SweepMultiByProfile(
			RawHits, Start, Location, FQuat::Identity,
			TraceConfig.CollisionProfile,
			FCollisionShape::MakeSphere(TraceConfig.Extent.X), Params);
		break;

	case EYSTraceShape::Capsule:
		bHit = World->SweepMultiByProfile(
			RawHits, Start, Location, Transform.GetRotation(),
			TraceConfig.CollisionProfile,
			FCollisionShape::MakeCapsule(TraceConfig.Extent.X, TraceConfig.Extent.Y), Params);
		break;

	case EYSTraceShape::Line:
		LineEnd = Location + Transform.GetUnitAxis(EAxis::X) * TraceConfig.Extent.X;
		bHit = World->LineTraceMultiByProfile(
			RawHits, Location, LineEnd,
			TraceConfig.CollisionProfile, Params);
		break;
	}

	PreviousSocketLocation = Location;
	bHasPreviousLocation = true;

	const float CurrentTime = World->GetTimeSeconds();
	TArray<FHitResult> ValidHits;

	if (bHit)
	{
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
	}

#if ENABLE_DRAW_DEBUG
	if (TraceConfig.bDebugDraw)
	{
		// 유효 히트 있음 → 빨강 / 충돌은 있지만 필터됨 → 주황 / 미충돌 → 초록
		const FColor ShapeColor = ValidHits.Num() > 0 ? FColor::Red
		                        : bHit               ? FColor::Orange
		                                             : FColor::Green;
		const float  Duration   = TraceConfig.DebugDrawDuration;
		const FQuat  Rotation   = Transform.GetRotation();

		switch (TraceConfig.Shape)
		{
		case EYSTraceShape::Box:
			DrawDebugBox(World, Start, TraceConfig.Extent, Rotation, ShapeColor, false, Duration);
			if (!Start.Equals(Location))
				DrawDebugBox(World, Location, TraceConfig.Extent, Rotation, ShapeColor, false, Duration);
			DrawDebugLine(World, Start, Location, ShapeColor, false, Duration, 0, 1.f);
			break;

		case EYSTraceShape::Sphere:
			DrawDebugSphere(World, Start, TraceConfig.Extent.X, 12, ShapeColor, false, Duration);
			if (!Start.Equals(Location))
				DrawDebugSphere(World, Location, TraceConfig.Extent.X, 12, ShapeColor, false, Duration);
			DrawDebugLine(World, Start, Location, ShapeColor, false, Duration, 0, 1.f);
			break;

		case EYSTraceShape::Capsule:
			// Extent.X = Radius, Extent.Y = HalfHeight
			DrawDebugCapsule(World, Start, TraceConfig.Extent.Y, TraceConfig.Extent.X, Rotation, ShapeColor, false, Duration);
			if (!Start.Equals(Location))
				DrawDebugCapsule(World, Location, TraceConfig.Extent.Y, TraceConfig.Extent.X, Rotation, ShapeColor, false, Duration);
			DrawDebugLine(World, Start, Location, ShapeColor, false, Duration, 0, 1.f);
			break;

		case EYSTraceShape::Line:
			DrawDebugLine(World, Location, LineEnd, ShapeColor, false, Duration, 0, 2.f);
			break;
		}

		for (const FHitResult& Hit : ValidHits)
		{
			DrawDebugSphere(World, Hit.ImpactPoint, 5.f, 8, FColor::Yellow, false, FMath::Max(Duration, 0.1f));
		}
	}
#endif

	if (ValidHits.Num() > 0)
	{
		OnTraceHit.Broadcast(ValidHits, TraceConfig.DamageRow);
	}
}
