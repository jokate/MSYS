// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "YSEnum.h"
#include "YSStruct.generated.h"

/**
 * 
 */

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
	FGameplayTagContainer InputTag;        // 인풋 바인딩

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
	
public :
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag TargetToTrigger;

	// 다음과 같이 선언된 사유는 간단, 다양한 페이로드 정보를 보낼 수 있다는 가정임.
	// (단 현재 어빌리티 내부에서 거의 처리되나, 만약 필요하면 선언해서 갈기자)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
	TArray<UYSAbilityTriggerPayload*> TriggerPayloads;
	
};

// 현재 해당 구조는 1 : 1 구조임.. (사실상 모든 걸 반영하기는 어려운 구조라면 구조가 맞음 )
// 일례로 기둥이 한번에 5개가 일렬로 파바바박 나가는 경우, 트레이스 형태는 Box가 맞지만, Extent는 30,30,30이 아닐 수 있음. (5개가 일렬로 나간다면 150,30,30이 될 수도..)
// 아니면 히트 데이터를 공유하는 방법 또한 존재하기는 함.
// 사실상 생각해보면, 각각의 GUID를 기준으로 데이터를 공유해도 될 거 같기는 한데.. 
USTRUCT(BlueprintType)
struct FYSTraceConfig
{
	GENERATED_BODY()

public :
	UPROPERTY(EditAnywhere, Category="YS | Sweep", meta=(DisplayName="데미지 정보"))
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

UENUM(BlueprintType)
enum class EYSPlaybackEvent : uint8
{
	None        UMETA(DisplayName = "첫 진입"),
	Completed   UMETA(DisplayName = "정상 완료"),
	Interrupted UMETA(DisplayName = "중단/취소"),
	OnHitTarget		UMETA(DisplayName = "타겟 히트 시"),
	OnHitPlayed		UMETA(DisplayName = "히트 당했을 시"),
	OnCheckContextTag		UMETA(DisplayName = "컨텍스트 태그 추가 시"),
	CustomEvent		UMETA(DisplayName = "커스텀 이벤트")
};

// 그래프를 흐르는 컨텍스트
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

	EYSPlaybackEvent PreviousResult = EYSPlaybackEvent::None;

	// 히트 발생 시 채워짐 (OnTraceComplete에서)
	TArray<FHitResult> HitResults;
	
	// 들어온 태그들에 대한 설정.
	UPROPERTY()
	FGameplayTagContainer ContextTags;
};

USTRUCT(BlueprintType)
struct FYSSpawnActorConfig
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Spawn", meta = (DisplayName = "스폰할 액터 클래스"))
	TSubclassOf<AActor> ActorClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Spawn | Position", meta = (DisplayName = "위치 기준 정책"))
	EYSPositionPolicy PositionPolicy = EYSPositionPolicy::UseSocket;

	// PositionPolicy == UseSocket 일 때 사용
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Spawn | Position",
		meta = (DisplayName = "소켓 이름", EditCondition = "PositionPolicy == EYSPositionPolicy::UseSocket", EditConditionHides))
	FName SpawnSocket = NAME_None;

	// PositionPolicy == UseRelativeOffset 일 때 사용
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Spawn | Position",
		meta = (DisplayName = "상대 오프셋",	EditCondition = "PositionPolicy == EYSPositionPolicy::UseRelativeOffset", EditConditionHides))
	FVector RelativeOffset = FVector::ZeroVector;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Spawn | Rotation", meta = (DisplayName = "회전 기준 정책"))
	EYSDirectionPolicy RotationPolicy = EYSDirectionPolicy::UseActorForwardVector;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Spawn | Rotation",
		meta = (DisplayName = "회전 소켓 이름 (빈칸이면 위치 소켓 공유)",
			EditCondition = "RotationPolicy == EYSDirectionPolicy::UseSocketRotation", EditConditionHides))
	FName RotationSocket = NAME_None;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Spawn", meta = (DisplayName = "어태치 여부"))
	bool bAttachToActor = false;
	
	// 해당 기능은 사실상 스포너가 위임할 예정.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Spawn", meta = (DisplayName = "스폰 딜레이 시간"))
	float SpawnDelay = 0.0f;
};