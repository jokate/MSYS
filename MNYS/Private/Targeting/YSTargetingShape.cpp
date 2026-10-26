// Fill out your copyright notice in the Description page of Project Settings.


#include "Targeting/YSTargetingShape.h"

#include "Targeting/YSSkillIndicator.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"

namespace
{
	void OverlapPawns(const UWorld* World, const FVector& Center, const FQuat& Rotation, const FCollisionShape& Shape, const AActor* Ignore, TArray<AActor*>& OutActors)
	{
		FCollisionQueryParams Params(SCENE_QUERY_STAT(YSTargetingShapeOverlap), false, Ignore);
		TArray<FOverlapResult> Overlaps;

		World->OverlapMultiByObjectType(Overlaps, Center, Rotation, FCollisionObjectQueryParams(ECC_Pawn), Shape, Params);

		for ( const FOverlapResult& Overlap : Overlaps )
		{
			if ( AActor* Actor = Overlap.GetActor() )
			{
				OutActors.AddUnique(Actor);
			}
		}
	}
}


void FYSTargetingShape::Draw(AYSSkillIndicator* Indicator, const FYSTargetingResult& Result, const FVector& CasterLocation) const
{
	if ( IsValid(Indicator) == false )
	{
		return;
	}

	// 링은 도형과 무관하게 시전자 발밑에 고정된다.
	Indicator->SetRangeRing(bShowRangeRing && GetRange() > 0.f, CasterLocation, GetRange());

	DrawShape(Indicator, Result, CasterLocation);
}

void FYSTargetingShape_Circle::DrawShape(AYSSkillIndicator* Indicator, const FYSTargetingResult& Result, const FVector& CasterLocation) const
{
	Indicator->ShowCircle(Result.Location, Radius);
}

void FYSTargetingShape_Square::DrawShape(AYSSkillIndicator* Indicator, const FYSTargetingResult& Result, const FVector& CasterLocation) const
{
	Indicator->ShowSquare(Result.Location, Length, Width);
}

void FYSTargetingShape_SelfCircle::DrawShape(AYSSkillIndicator* Indicator, const FYSTargetingResult& Result, const FVector& CasterLocation) const
{
	Indicator->ShowCircle(CasterLocation, Radius);
}

void FYSTargetingShape_Line::DrawShape(AYSSkillIndicator* Indicator, const FYSTargetingResult& Result, const FVector& CasterLocation) const
{
	Indicator->ShowLine(CasterLocation, Result.Direction.Rotation().Yaw, Length, Width);
}

void FYSTargetingShape_Cone::DrawShape(AYSSkillIndicator* Indicator, const FYSTargetingResult& Result, const FVector& CasterLocation) const
{
	Indicator->ShowCone(CasterLocation, Result.Direction.Rotation().Yaw, Radius, AngleDegrees);
}

bool FYSTargetingShape_SelfCircle::CollectActors(const UWorld* World, const FVector& Origin, const FVector& Forward, const AActor* Ignore, TArray<AActor*>& OutActors) const
{
	if ( IsValid(World) == false )
	{
		return false;
	}

	OverlapPawns(World, Origin, FQuat::Identity, FCollisionShape::MakeSphere(Radius), Ignore, OutActors);
	return true;
}

bool FYSTargetingShape_Line::CollectActors(const UWorld* World, const FVector& Origin, const FVector& Forward, const AActor* Ignore, TArray<AActor*>& OutActors) const
{
	if ( IsValid(World) == false )
	{
		return false;
	}

	const FVector Direction = Forward.GetSafeNormal2D();
	const float HalfLength = FMath::Max(Length * 0.5f, Width * 0.5f);

	// 캡슐 축(Z)을 전방에 맞춘다.
	const FQuat Rotation = FRotationMatrix::MakeFromZ(Direction).ToQuat();
	OverlapPawns(World, Origin + Direction * HalfLength, Rotation, FCollisionShape::MakeCapsule(Width * 0.5f, HalfLength), Ignore, OutActors);
	return true;
}

bool FYSTargetingShape_Cone::CollectActors(const UWorld* World, const FVector& Origin, const FVector& Forward, const AActor* Ignore, TArray<AActor*>& OutActors) const
{
	if ( IsValid(World) == false )
	{
		return false;
	}

	TArray<AActor*> InRadius;
	OverlapPawns(World, Origin, FQuat::Identity, FCollisionShape::MakeSphere(Radius), Ignore, InRadius);

	// 각도는 인디케이터와 같이 전체 각이다.
	const FVector Direction = Forward.GetSafeNormal2D();
	const float CosHalfAngle = FMath::Cos(FMath::DegreesToRadians(AngleDegrees * 0.5f));

	for ( AActor* Actor : InRadius )
	{
		const FVector ToActor = (Actor->GetActorLocation() - Origin).GetSafeNormal2D();

		if ( ToActor.IsNearlyZero() || FVector::DotProduct(Direction, ToActor) >= CosHalfAngle )
		{
			OutActors.AddUnique(Actor);
		}
	}

	return true;
}
