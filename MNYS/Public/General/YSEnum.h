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
enum class EYSDirectionPolicy : uint8
{
	UseActorForwardVector UMETA(DisplayName = "액터 전방 벡터 사용"),
	UseTowardPlaybackTarget 	UMETA(DisplayName = "플레이 백 목표를 향하는 방향 사용"),
	UseTowardLockOnTarget		UMETA(DisplayName = "락온 목표를 향하는 방향 사용"),
	UseControlRotation			UMETA(DisplayName = "컨트롤 로테이션 기준"),
	UseTowardIndicatorPosition	UMETA(DisplayName = "인디케이터 포지션 사용."),
	UseSocketRotation			UMETA(DisplayName = "소켓 포지션 사용"),
	UseRelativeOffset			UMETA(DisplayName = "액터 기준 상대 오프셋")
};

/*
 * 위치(Position)와 회전(Rotation)을 독립적으로 결정하기 위해 분리합니다.
 */
UENUM(BlueprintType)
enum class EYSPositionPolicy : uint8
{
	UseActorLocation	UMETA(DisplayName = "액터 위치"),
	UseSocket			UMETA(DisplayName = "소켓 위치"),
	UseRelativeOffset	UMETA(DisplayName = "액터 기준 상대 오프셋"),
};

// 스킬 슬롯에 세팅할 수 있게끔 처리할 예정 ( Subclass로 판단 할 수 있도록 ) / 내 생각에는 기준이 되는 애셋 잡고, 이걸 기반으로 설정가능하도록 해야 할 듯.
UENUM(BlueprintType)
enum class EYSSkillType : uint8
{
	Basic,
	Special,
	JustAvoid,
};

UENUM(BlueprintType)
enum class EYSAttackActivationType : uint8
{
	Instant			UMETA(DisplayName = "즉시 발동"),
	TimeBased		UMETA(DisplayName = "시간 기반"),
	TagBased		UMETA(DisplayName = "태그 기반"),
};
