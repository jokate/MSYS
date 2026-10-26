// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/YSSwitchWindowCloseCondition.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

bool FYSSwitchWindowCloseCondition::CanCloseEarly(const FYSSwitchWindowCloseContext& Context) const
{
	return Context.OpenedFor >= MinOpenTime && ShouldClose(Context);
}

bool FYSSwitchWindowCloseCondition_ControlYawAligned::ShouldClose(const FYSSwitchWindowCloseContext& Context) const
{
	// 대상을 잃으면 끝날 회전도 없다. 상한(GE 지속)에 맡긴다.
	if ( IsValid(Context.Controller) == false || IsValid(Context.Pawn) == false || IsValid(Context.Target) == false )
	{
		return false;
	}

	const FVector ToTarget = Context.Target->GetActorLocation() - Context.Pawn->GetActorLocation();
	const float TargetYaw = ToTarget.Rotation().Yaw;
	const float ControlYaw = Context.Controller->GetControlRotation().Yaw;

	return FMath::Abs(FMath::FindDeltaAngleDegrees(ControlYaw, TargetYaw)) <= YawTolerance;
}
