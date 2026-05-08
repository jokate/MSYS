// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "YSEnum.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EYSInputStatesType : uint8
{
	None = 0,
	Idle,
	Attack,
	Dodge,
	JustAvoid,
	Falling,
	Skill,
	Ready,
	End,
};

UENUM(BlueprintType)
enum class EVelocityDirection : uint8
{
	Frontward,
	Backward,
	Left,
	Right,
	FrontwardLeft,
	BackwardLeft,
	FrontwardRight,
	BackwardRight
};

UENUM(BlueprintType)
enum class EMotionWarpType : uint8
{
	TranslationAndRotation,
	TranslationOnly,
	RotationOnly,
	NoneMotionWarp,
};