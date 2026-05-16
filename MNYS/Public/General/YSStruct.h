// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "YSEnum.h"
#include "YSStruct.generated.h"

/**
 * 
 */

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
	TEnumAsByte<ECollisionChannel> Channel = ECC_Pawn;

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
};
