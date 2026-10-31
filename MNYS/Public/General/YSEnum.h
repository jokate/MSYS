// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "YSEnum.generated.h"

/* 입력의 위상. 태그 접미사로 넣으면 State × Input × 위상 조합이 불어나 별도 축으로 흘린다. ETriggerEvent 를 직접 노출하지 않는 것은 무의미한 선택지를 드롭다운에서 지우고 Enhanced Input 과의 결합을 끊기 위해서다. */
UENUM(BlueprintType)
enum class EYSInputPhase : uint8
{
	Pressed	= 0	UMETA(DisplayName = "누름"),			// ETriggerEvent::Started
	// 홀드는 사건이 아니라 지금 눌린 상태라 소비 개념이 없다. IsInputHeld 로 질의하고 이 값은 더 이상 발행되지 않는다. 지우지 않고 숨기는 것은 뒤 항목의 정수 값이 밀려 에셋이 깨지기 때문이다.
	Held		UMETA(Hidden),
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

// 스킬 슬롯에 세팅할 수 있게끔 처리할 예정 (Subclass 로 판단할 수 있도록).
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
	Any,
	Hostile,
	Friendly,
	Neutral,
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

UENUM(BlueprintType)
enum class EYSOperatorType : uint8
{
	OR,
	AND
};

// 경계값은 UYSCombatProfile 이 정한다. 가까운 순서로 둔다 — 이력(떨림 방지) 판정이 이 순서를 비교에 쓴다.
UENUM(BlueprintType)
enum class EYSTargetDistanceBand : uint8
{
	Near		UMETA(DisplayName = "근"),
	Mid			UMETA(DisplayName = "중"),
	Far			UMETA(DisplayName = "원"),
	OutOfRange	UMETA(DisplayName = "구간 밖 / 타겟 없음"),
};

// 타겟이 나보다 어디 있는가. 경계는 UYSDeveloperSettings 의 타겟 구간 경계.
UENUM(BlueprintType)
enum class EYSTargetHeightBand : uint8
{
	Below	UMETA(DisplayName = "아래"),
	Level	UMETA(DisplayName = "같음"),
	Above	UMETA(DisplayName = "위"),
};

// 기준 정면과 타겟 방향의 각. 경계는 UYSDeveloperSettings 의 타겟 구간 경계.
UENUM(BlueprintType)
enum class EYSTargetDirectionBand : uint8
{
	Front	UMETA(DisplayName = "정면"),
	Side	UMETA(DisplayName = "측면"),
	Rear	UMETA(DisplayName = "후면"),
};

UENUM(BlueprintType)
enum class EYSForwardReference : uint8
{
	Current		UMETA(DisplayName = "지금 정면"),
	PreSwitch	UMETA(DisplayName = "전환 직전 정면"),
};

// 타겟이 바뀐 까닭. 락온 카메라가 따라갈지를 이것으로 정한다.
UENUM(BlueprintType)
enum class EYSTargetChangeReason : uint8
{
	Auto	UMETA(DisplayName = "자동 전환"),
	Manual	UMETA(DisplayName = "수동 전환 (Q/E)"),
	Engage	UMETA(DisplayName = "교전 시작 자동 타겟"),
	Forced	UMETA(DisplayName = "강제 락온 (GE)"),
};

// 몸 회전을 요청한 쪽. 출처마다 따르는 규칙은 전투 프로필의 출처별 규칙 표에 있다.
UENUM(BlueprintType)
enum class EYSFaceYawSource : uint8
{
	TargetSwitch	UMETA(DisplayName = "타겟 전환 · 락온"),
	AbilityMotion	UMETA(DisplayName = "어빌리티 이동 이벤트"),
};

UENUM(BlueprintType)
enum class EYSSwitchTrigger : uint8
{
	TargetSwitch	UMETA(DisplayName = "타겟 전환"),
	TagSwap			UMETA(DisplayName = "태그 교대"),
};

// 어느 트리거가 스위치 창을 여는가. 기획 미결이라 설정 값으로 둔다.
UENUM(BlueprintType)
enum class EYSSwitchWindowTriggerRule : uint8
{
	Both				UMETA(DisplayName = "둘 다"),
	TargetSwitchOnly	UMETA(DisplayName = "타겟 전환만"),
	TagSwapOnly			UMETA(DisplayName = "태그 교대만"),
};

// 스위치 남용 기록(쿨 · 감쇠 · 연속 소모)을 트리거끼리 나눠 쓰는가.
UENUM(BlueprintType)
enum class EYSSwitchAbuseScope : uint8
{
	PerTrigger	UMETA(DisplayName = "트리거별"),
	Shared		UMETA(DisplayName = "공유"),
};

// 스위치 어택 진입기 형이 판정 도형 안에서 누구를 맞히는가.
UENUM(BlueprintType)
enum class EYSSwitchEntryTargetRule : uint8
{
	CurrentTarget	UMETA(DisplayName = "현재 타겟 1체"),
	AllInShape		UMETA(DisplayName = "도형 안 전부"),
};

// 스위치 어택 진입기 형의 이동.
UENUM(BlueprintType)
enum class EYSSwitchEntryMove : uint8
{
	RootMotion		UMETA(DisplayName = "모션 루트모션 그대로"),
	WarpToTarget	UMETA(DisplayName = "타겟 앞까지 모션 워프"),
};

UENUM(BlueprintType)
enum class EYSStaggerGrade : uint8
{
	Soft		UMETA(DisplayName = "0 — 무른 (잡몹)"),
	Firm		UMETA(DisplayName = "1 — 단단한 (정예)"),
	Immovable	UMETA(DisplayName = "2 — 부동 (엘리트 · 보스)"),
};

// 등급 하나의 값(시간 · 셰이크 · 줌)은 YS Developer Settings 의 임팩트 행이 갖는다. None 은 임팩트 없음.
UENUM(BlueprintType)
enum class EYSImpactGrade : uint8
{
	S		UMETA(DisplayName = "S"),
	M		UMETA(DisplayName = "M"),
	MPlus	UMETA(DisplayName = "M+"),
	L		UMETA(DisplayName = "L"),
	LPlus	UMETA(DisplayName = "L+"),
	None	UMETA(DisplayName = "없음"),
};

// 브레이크 연출 단계. 단계가 겹칠 수 있다 — 무방비는 진입 · 예고를 감싼다.
UENUM(BlueprintType)
enum class EYSBreakPhase : uint8
{
	Enter		UMETA(DisplayName = "진입 (정지 · 카메라)"),
	Broken		UMETA(DisplayName = "무방비"),
	Warning		UMETA(DisplayName = "회복 예고"),
	Grace		UMETA(DisplayName = "회복 뒤 유예"),
};

// 연결 액터가 해제되는 사유. 어떻게 사라질지(폭발/무피해)는 액터가 사유를 보고 정한다.
UENUM(BlueprintType)
enum class EYSReleaseReason : uint8
{
	Command		UMETA(DisplayName = "명령"),
	Expired		UMETA(DisplayName = "만료"),
	Cancelled	UMETA(DisplayName = "어빌리티 취소")
};

// 시간 왜곡을 누구에게 거는가. 기준은 큐 대상이다.
UENUM(BlueprintType)
enum class EYSTimeDilationScope : uint8
{
	All				UMETA(DisplayName = "전원"),
	HostileToTarget	UMETA(DisplayName = "큐 대상의 적만"),
	Participants	UMETA(DisplayName = "때린 쪽 · 맞은 쪽만"),
};

UENUM(BlueprintType)
enum class EYSCueActorSelector : uint8
{
	CueTarget	UMETA(DisplayName = "큐 대상"),
	Instigator	UMETA(DisplayName = "시전자 (저스트 회피면 판정 적)"),
};