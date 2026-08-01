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

/*
 * 입력의 위상(누름/홀드/뗌)입니다.
 * 태그(State.Input.Action)에 접미사로 넣지 않고 별도 축으로 흘리는 이유는,
 * State 11종 × Input 13종에 위상까지 곱하면 조합이 286개를 넘어가는데
 * 실제로 어빌리티가 물리는 조합은 그중 일부뿐이기 때문입니다.
 *
 * ETriggerEvent를 직접 노출하지 않는 것은 None/Ongoing 같은 무의미한 선택지를
 * 데이터 에셋 드롭다운에서 지우고, Enhanced Input과의 결합을 끊기 위함입니다.
 */
UENUM(BlueprintType)
enum class EYSInputPhase : uint8
{
	Pressed		UMETA(DisplayName = "누름"),			// ETriggerEvent::Started
	Held		UMETA(DisplayName = "누르는 중"),		// ETriggerEvent::Triggered (매 프레임)
	Released	UMETA(DisplayName = "뗌"),				// ETriggerEvent::Completed
	Canceled	UMETA(DisplayName = "취소"),			// ETriggerEvent::Canceled
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
	Line,
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
	RandomizedPosition	UMETA(DisplayName = "특정 범위에 대한 랜덤 포지선")
};

// 스킬 슬롯에 세팅할 수 있게끔 처리할 예정 ( Subclass로 판단 할 수 있도록 ) / 내 생각에는 기준이 되는 애셋 잡고, 이걸 기반으로 설정가능하도록 해야 할 듯.
UENUM(BlueprintType)
enum class EYSSkillType : uint8
{
	Basic		UMETA(DisplayName = "기본"),
	Special		UMETA(DisplayName = "특수"),
	JustAvoid	UMETA(DisplayName = "회피"),
	Movement	UMETA(DisplayName = "이동")
};

UENUM(BlueprintType)
enum class EYSAttackActivationType : uint8
{
	Instant			UMETA(DisplayName = "즉시 발동"),
	TimeBased		UMETA(DisplayName = "시간 기반"),
	TagBased		UMETA(DisplayName = "태그 기반"),
};


UENUM()
enum class EYSPerceptionAffiliation : uint8
{
	Any,       // 전부
	Hostile,   // 적만 (적이 낸 소리/움직임만)
	Friendly,  // 아군만
	Neutral,   // 중립만
};

UENUM(BlueprintType)
enum class EYSTargetingPolicy : uint8
{
	Fixed,
	Dynamic,
};

UENUM(BlueprintType)
enum class EYSScoreObjectType : uint8
{
	Owner,
	Target
};

UENUM(BlueprintType)
enum class EYSIAUSType : uint8
{	
	LinearQuadratic	UMETA(DisplayName = "선형/이차"),
	Logistic UMETA(DisplayName = "로지스틱"),
	Logit	UMETA(DisplayName = "로짓"),
	Gaussian UMETA(DisplayName = "가우시안")
};