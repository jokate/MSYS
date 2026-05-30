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

UENUM(BlueprintType)
enum class EYSVelocityDirectionPolicy : uint8
{
	UseActorForwardVector UMETA(DisplayName = "액터 전방 벡터 사용"),
	UseTowardPlaybackTarget 	UMETA(DisplayName = "플레이 백 목표를 향하는 방향 사용"),
	UseTowardLockOnTarget		UMETA(DisplayName = "락온 목표를 향하는 방향 사용"),
	UseControlRotation			UMETA(DisplayName = "컨트롤 로테이션 기준"),
	UseTowardIndicatorPosition	UMETA(DisplayName = "인디케이터 포지션 사용.")
};