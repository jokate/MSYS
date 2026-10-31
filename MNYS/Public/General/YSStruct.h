// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "YSEnum.h"
#include "Data/YSTransformPolicy.h"
#include "YSStruct.generated.h"


class AYSTelegraphActor;
struct FGameplayEventData;
class UCameraShakeBase;
class ULevelSequence;
class UYSAbilityTriggerPayload;
class UYSGameplayAbility;



USTRUCT(BlueprintType)
struct FYSGrantedAbilityData
{
	GENERATED_BODY()

public : 
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UYSGameplayAbility> AbilityClass;
	
	UPROPERTY(EditDefaultsOnly)
	FGameplayTagContainer InputTag;
	
	UPROPERTY(EditDefaultsOnly)
	EYSSkillType SkillType = EYSSkillType::Basic;

	UPROPERTY(EditDefaultsOnly)
	int32 Level = 1;
};

USTRUCT(BlueprintType)
struct FGameplayEventSendData
{
	GENERATED_BODY()

public :
	FGameplayEventSendData()
	{
		TriggerPayloads.SetNum(2);
	}
	
	bool IsValid() const
	{
		return TargetToTrigger.IsValid();
	}

	FGameplayEventData MakeEventData(AActor* Instigator, AActor* Target = nullptr) const;

	/** 액터 자신이 보낸 이벤트로 그 액터의 ASC 에 보낸다 (애님 노티파이용). */
	void SendToActor(AActor* Actor) const;

public :
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag TargetToTrigger;

	// 다양한 페이로드를 보낼 수 있게 열어 둔 칸 (현재는 어빌리티 내부에서 거의 처리).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
	TArray<UYSAbilityTriggerPayload*> TriggerPayloads;
	
};

// 트레이스 설정은 1 : 1 구조라, 여러 개가 한 번에 나가는 공격(기둥 5개 일렬 등)의 Extent 를 다 담지 못한다.
USTRUCT(BlueprintType)
struct FYSTraceConfig
{
	GENERATED_BODY()

public :
	UPROPERTY(EditAnywhere, Category="YS | Sweep", meta=(DisplayName="데미지 정보", GetOptions="MNYS.YSDeveloperSettings.GetDamageRowOptions"))
	FName DamageRow;
	
	UPROPERTY(EditAnywhere, Category="YS | Sweep", meta=(DisplayName="트레이스 형태"))
	EYSTraceShape Shape = EYSTraceShape::Box;

	// 박스면 전체 사용 / 구, 라인 = 앞의 값만 / 캡슐이면 XY 사용.
	UPROPERTY(EditAnywhere, Category="YS | Sweep", meta=(DisplayName="크기"))
	FVector Extent = FVector(30.f, 30.f, 30.f);
	
	UPROPERTY(EditAnywhere, Category="YS | Sweep", meta=(DisplayName="충돌 채널"))
	FName CollisionProfile = NAME_None;

	UPROPERTY(EditAnywhere, Category="YS | Sweep", meta=(DisplayName="기준 소켓"))
	FName SocketName = NAME_None;

	UPROPERTY(EditAnywhere, Category="YS | Sweep", meta=(DisplayName="상대 위치"))
	FVector RelativeLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category="YS | Sweep", meta=(DisplayName="한번만 트레이스 할 것인가?"))
	bool bTraceOnce = false;

	UPROPERTY(EditAnywhere, Category="YS | Sweep", meta=(DisplayName="연속 히트 판정 가능"))
	bool bContinuousHit = false;

	UPROPERTY(EditAnywhere, Category="YS | Sweep", meta=(DisplayName="히트 타이머", EditCondition="bContinuousHit", EditConditionHides))
	float HitTimeCheck = 0.1f;
	
	// 0인 경우 개수 제한 X / 0보다 크면 히트카운트 존재.
	UPROPERTY(EditAnywhere, Category="YS | Sweep", meta=(DisplayName="히트 카운트"))
	int32 LimitHitCount = 0;
	
	UPROPERTY(EditAnywhere, Category="YS | Sweep | Debug", meta=(DisplayName="디버그 드로우"))
	bool bDebugDraw = false;
	
	// 0이면 단일 프레임, TraceOnce 사용 시 1.0 이상 권장
	UPROPERTY(EditAnywhere, Category="YS | Sweep | Debug", meta=(DisplayName="드로우 유지 시간(초)", EditCondition="bDebugDraw", EditConditionHides))
	float DebugDrawDuration = 0.f;
};


USTRUCT(BlueprintType)
struct FYSSequencePlaySettings
{
	GENERATED_BODY()
	
public : 
	UPROPERTY(EditDefaultsOnly, Category = "YS | Sequence", meta = (DisplayName = "재생할 레벨 시퀀스"))
	TSoftObjectPtr<ULevelSequence> Sequence;
	
	UPROPERTY(EditDefaultsOnly, Category = "YS | Sequence",	meta = (DisplayName = "카메라 컷 오버라이드 사용"))
	bool bOverrideCameraBySequence = true;
	
	UPROPERTY(EditDefaultsOnly, Category = "YS | Sequence",	meta = (DisplayName = "재생 속도", ClampMin = "0.1"))
	float PlayRate = 1.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "YS | Sequence", meta = (DisplayName = "선행 몽타주 블렌드아웃 시간", ClampMin = "0.0", ClampMax = "0.5"))
	float MontageBlendOutTime = 0.15f;
};

USTRUCT(BlueprintType)
struct FYSInputSaveDataMemeber
{
	GENERATED_BODY()
	
public : 
	FYSInputSaveDataMemeber() = default;
	
	FYSInputSaveDataMemeber(const FGameplayTagContainer& InputTags, TSubclassOf<UYSGameplayAbility> InAbilityClass)
	{
		InputGameplayTag = InputTags;
		AbilityClass = InAbilityClass;
	}
	
	bool operator==(const FYSInputSaveDataMemeber& Other) const
	{
		return InputGameplayTag == Other.InputGameplayTag;
	}
	
	UPROPERTY()
	FGameplayTagContainer InputGameplayTag;
	
	UPROPERTY()
	TSubclassOf<UYSGameplayAbility> AbilityClass;
};

USTRUCT(BlueprintType)
struct FYSTagHistory
{
	GENERATED_BODY()

	FYSTagHistory() = default;
	
	FYSTagHistory(const FGameplayTag& InInputTag, float InInputTime = 0.f)
		: InputTag(InInputTag), InputTime(InInputTime)
	{
	}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag InputTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InputTime;
};


USTRUCT(BlueprintType)
struct FYSInputHistory : public FYSTagHistory
{
	GENERATED_BODY()
	
public : 
	FYSInputHistory() = default;
	FYSInputHistory(const FGameplayTag& InInputTag, EYSInputPhase InInputPhase = EYSInputPhase::Pressed, float InInputTime = 0.f)
		: FYSTagHistory{InInputTag, InInputTime}, InputPhase(InInputPhase)
	{
	}
	
	bool operator==(const FYSInputHistory& Other) const
	{
		return InputTag == Other.InputTag && InputPhase == Other.InputPhase;
	}
	
	UPROPERTY(EditDefaultsOnly, meta = (DisplayName="인풋 페이즈"))
	EYSInputPhase InputPhase = EYSInputPhase::Pressed;
};

USTRUCT(BlueprintType)
struct FYSPlaybackContext
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UYSGameplayAbility> OwnerAbility;

	UPROPERTY()
	TObjectPtr<AActor> Instigator;

	UPROPERTY()
	TObjectPtr<AActor> Target;

	UPROPERTY()
	FGameplayTagContainer ContextTags;
};

/** 타겟 높이 · 방향 구간의 경계. 재생 조건이 같이 쓴다. */
USTRUCT()
struct MNYS_API FYSTargetBandRules
{
	GENERATED_BODY()

	// 타겟 Z − 내 Z 가 +이 값을 넘으면 위, −이 값보다 낮으면 아래.
	UPROPERTY(EditAnywhere, Category = "YS | Target", meta = (DisplayName = "높이 경계 (cm)", ClampMin = "0"))
	float HeightThreshold = 100.f;

	UPROPERTY(EditAnywhere, Category = "YS | Target", meta = (DisplayName = "정면 각 상한 (도)", ClampMin = "0", ClampMax = "180"))
	float FrontAngle = 60.f;

	// 이 각을 넘으면 후면.
	UPROPERTY(EditAnywhere, Category = "YS | Target", meta = (DisplayName = "측면 각 상한 (도)", ClampMin = "0", ClampMax = "180"))
	float SideAngle = 120.f;

	EYSTargetHeightBand ResolveHeight(float DeltaZ) const;
	EYSTargetDirectionBand ResolveDirection(const FVector& Forward, const FVector& ToTarget) const;
};

USTRUCT(BlueprintType)
struct FYSSpawnActorConfig
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Spawn", meta = (DisplayName = "스폰할 액터 클래스"))
	TSubclassOf<AActor> ActorClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Spawn", meta = (DisplayName = "Transform 정책"))
	FYSTransformPolicy TransformPolicy;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Spawn", meta = (DisplayName = "어태치 여부"))
	bool bAttachToActor = false;
	
	// 초. 기다리는 동안 요청한 자리에 예고 표시가 뜬다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Spawn", meta = (DisplayName = "스폰 딜레이 시간"))
	float SpawnDelay = 0.0f;

	// 끄면 낸 어빌리티가 끝나도 지연 스폰은 나온다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Spawn", meta = (DisplayName = "어빌리티가 끝나면 지연 스폰 취소"))
	bool bCancelOnAbilityEnd = false;

	// 프로젝타일의 경우에는 공중에 스폰되어도 무관하나, 만약 땅에 부조건 붙어야 하는 경우에는 무조건 필요.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Spawn", meta = (DisplayName = "땅에 붙어서 스폰해야 하는 경우"))
	bool bStickGround = false;
};

USTRUCT(BlueprintType)
struct FYSCameraEffectParams
{
	GENERATED_BODY()
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Camera", meta = (DisplayName = "목표 암 길이 (줌인)"))
	float TargetArmLength = 250.f;
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Camera", meta = (DisplayName = "소켓 오프셋 Z (로우앵글)"))
	FVector SocketOffset = FVector( 0, 0, -80.f);
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Camera", meta = (DisplayName = "목표 FOV"))
	float FieldOfView = 70.f;
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Camera", meta = (DisplayName = "진입 보간 속도"))
	float InterpInSpeed = 10.f;
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Camera", meta = (DisplayName = "복귀 보간 속도"))
	float InterpOutSpeed = 5.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Camera", meta = (DisplayName = "카메라 상대 회전 (스프링암 기준)"))
	FRotator RelativeRotator = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Camera", meta = (DisplayName = "컨트롤 회전에 맞추기"))
	bool bOrientToControlRotation = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Camera", meta = (DisplayName = "Yaw 회전"))
	bool bUseControllerRotationYaw = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Camera", meta = (DisplayName = "컨트롤 피치 고정 사용"))
	bool bOverrideControlPitch = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Camera",
		meta = (DisplayName = "고정할 컨트롤 피치 (음수 = 내려다봄)", EditCondition = "bOverrideControlPitch", EditConditionHides, ClampMin = "-89.0", ClampMax = "89.0"))
	float ControlPitch = -55.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Camera",
		meta = (DisplayName = "도달 후 피치 입력 허용", EditCondition = "bOverrideControlPitch", EditConditionHides))
	bool bFreeControlPitchAfterReach = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Camera", meta = (DisplayName = "컨트롤 요 고정 사용"))
	bool bLockControlYaw = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Camera", meta = (DisplayName = "커서 방향 카메라 이동 사용"))
	bool bUseCursorCameraLean = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Camera",
		meta = (DisplayName = "커서 카메라 최대 이동 거리", EditCondition = "bUseCursorCameraLean", EditConditionHides, ClampMin = "0.0"))
	float CursorLeanDistance = 700.f;

	/** 이 반경 안에서는 카메라가 반응하지 않는다. 0이면 커서를 조금만 흔들어도 화면이 따라 흔들린다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Camera",
		meta = (DisplayName = "커서 카메라 데드존", EditCondition = "bUseCursorCameraLean", EditConditionHides, ClampMin = "0.0", ClampMax = "0.99"))
	float CursorLeanDeadZone = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Camera",
		meta = (DisplayName = "커서 카메라 이동 보간 속도", EditCondition = "bUseCursorCameraLean", EditConditionHides, ClampMin = "0.1"))
	float CursorLeanInterpSpeed = 7.f;
};

/** 임팩트 등급 하나의 시간 · 셰이크 · 줌 강도. 어느 행을 쓰는지는 피해 행의 임팩트 칸이 정한다. */
USTRUCT(BlueprintType)
struct FYSImpactProfile
{
	GENERATED_BODY()

	// 0 = 정지(히트스톱), 0~1 = 슬로우.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Impact | Time", meta = (DisplayName = "시간 배율", ClampMin = "0", ClampMax = "1"))
	float TimeDilation = 0.f;

	// 0 이면 시간 채널을 쓰지 않는다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Impact | Time", meta = (DisplayName = "시간 길이 (초)", ClampMin = "0"))
	float TimeDuration = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Impact | Time", meta = (DisplayName = "시간 범위"))
	EYSTimeDilationScope TimeScope = EYSTimeDilationScope::All;

	// 비우면 셰이크 없음.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Impact | Shake", meta = (DisplayName = "셰이크"))
	TSubclassOf<UCameraShakeBase> ShakeClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Impact | Shake", meta = (DisplayName = "셰이크 배율", ClampMin = "0"))
	float ShakeScale = 1.f;

	// 0 이면 줌 없음. 줌은 임팩트 우선순위로 카메라 모드 스택에 얹혔다가 이 시간 뒤 빠진다. 아래 거리 · 높이 · 화각은 줌 순간 카메라에 대한 증감이다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Impact | Zoom", meta = (DisplayName = "줌 유지 (초)", ClampMin = "0"))
	float ZoomDuration = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Impact | Zoom", meta = (DisplayName = "줌 거리 증감 (cm, 음수 = 가까이)", EditCondition = "ZoomDuration > 0", EditConditionHides))
	float ZoomArmLengthDelta = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Impact | Zoom", meta = (DisplayName = "줌 높이 증감 (cm, 소켓 오프셋 Z)", EditCondition = "ZoomDuration > 0", EditConditionHides))
	float ZoomHeightDelta = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Impact | Zoom", meta = (DisplayName = "줌 화각 증감 (도)", EditCondition = "ZoomDuration > 0", EditConditionHides))
	float ZoomFOVDelta = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Impact | Zoom", meta = (DisplayName = "줌 진입 보간 속도", ClampMin = "0.1", EditCondition = "ZoomDuration > 0", EditConditionHides))
	float ZoomInterpInSpeed = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Impact | Zoom", meta = (DisplayName = "줌 복귀 보간 속도", ClampMin = "0.1", EditCondition = "ZoomDuration > 0", EditConditionHides))
	float ZoomInterpOutSpeed = 5.f;
};

USTRUCT(BlueprintType)
struct FYSTargetingActorCollections
{
	GENERATED_BODY()
	
public : 
	FYSTargetingActorCollections() {}
	FYSTargetingActorCollections(AActor* Owner) : OwnerActor(Owner) {}
	
	AActor* GetOwnerActor() const { return OwnerActor.Get(); }
	AActor* GetBestTargetActor() const { return BestTargetActor.Get(); }
	void ResetBestTargetActor() { BestTargetActor.Reset(); }
	TArray<AActor*> GetSkillTargetActors() const
	{
		TArray<AActor*> TargetActors;
		for ( TWeakObjectPtr<AActor> SkillTargetActor : SkillTargetActors )
		{
			AActor* TempSkillTarget = SkillTargetActor.Get();
			
			if ( IsValid(TempSkillTarget) == false )
			{
				continue;
			}
			
			TargetActors.Emplace(TempSkillTarget);
		}
		
		return TargetActors;
	}
	
	void SetBestTargetActor(AActor* TargetActor)
	{
		BestTargetActor = TargetActor;
	}
	
	void AddSkillTargetActor(AActor* SkillTargetActor)
	{
		SkillTargetActors.AddUnique(SkillTargetActor);
	}
	
private :
	TWeakObjectPtr<AActor> OwnerActor;
	
	TWeakObjectPtr<AActor> BestTargetActor;
	
	// 스킬에 대한 타겟팅 지정이 완료되었다면 해당 부분에 대해서 활용.
	TArray<TWeakObjectPtr<AActor>> SkillTargetActors;
};

class UAnimMontage;
class UCurveFloat;

/** 몸 회전 각도 구간 1행. 회전각이 MaxAngle 이하인 첫 행을 쓴다. 몽타주가 없으면 Duration / PlayRate 동안 보간한다. */
USTRUCT(BlueprintType)
struct FYSTurnBand
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "최대각 (도)", ClampMin = "0", ClampMax = "180"))
	float MaxAngle = 180.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "보간 시간 (초)", ClampMin = "0"))
	float Duration = 0.15f;

	// 0~1 입력 · 0~1 출력. 비면 SmoothStep.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "보간 커브"))
	TObjectPtr<UCurveFloat> Curve;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "재생 배율", ClampMin = "0.01"))
	float PlayRate = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "턴 몽타주 (왼쪽)"))
	TObjectPtr<UAnimMontage> MontageLeft;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "턴 몽타주 (오른쪽)"))
	TObjectPtr<UAnimMontage> MontageRight;
};

USTRUCT(BlueprintType)
struct FYSTurnSourceRule
{
	GENERATED_BODY()

	// 켜면 막는 상태 태그가 있는 동안 요청을 1칸 대기시키고, 태그가 빠지면 돈다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "막는 태그 따름"))
	bool bRespectBlockedTags = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "턴 몽타주 허용"))
	bool bAllowTurnMontage = true;

	// 보간 시간 상한. 0 이면 상한 없음.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "최대 시간 (초, 0=없음)", ClampMin = "0"))
	float MaxDuration = 0.f;
};
