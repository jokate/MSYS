// Fill out your copyright notice in the Description page of Project Settings.


#include "Targeting/YSTargetingShape.h"

#include "Targeting/YSSkillIndicator.h"


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
