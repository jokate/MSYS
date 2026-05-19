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
	JumpAttack,
	Damaged,
	Death,
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
	None,
	TranslationAndRotation UMETA(DisplayName = "이동 + 회전"),
	TranslationOnly UMETA(DisplayName = "이동만"),
	RotationOnly UMETA(DisplayName = "회전만"),
};

UENUM(BlueprintType)
enum class EYSTraceShape : uint8
{
	Box,
	Sphere,
	Capsule,
	Line
};