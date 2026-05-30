// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GameFramework/RootMotionSource.h"
#include "General/YSStruct.h"
#include "UObject/Object.h"
#include "YSAbilityTriggerPayload.generated.h"

struct FGameplayEventData;
/**
 * Trigger Payload로 보내고자 하는 경우에 대해서 전제 처리.
 */
UCLASS(BlueprintType, Blueprintable, DefaultToInstanced, EditInlineNew )
class MNYS_API UYSAbilityTriggerPayload : public UObject
{
	GENERATED_BODY()
	
	
public : 
	template<typename T>
	static const T* GetPayload(const FGameplayEventData* EventData);
	
public :
	UPROPERTY()
	TWeakObjectPtr<AActor> OwnerActor;
};

template <typename T>
const T* UYSAbilityTriggerPayload::GetPayload(const FGameplayEventData* EventData)
{
	if ( EventData->OptionalObject == nullptr && EventData->OptionalObject2 == nullptr )
		return nullptr;
	
	const T* PayloadData = Cast<T>(EventData->OptionalObject);

	return IsValid(PayloadData) ? PayloadData : Cast<T>(EventData->OptionalObject2);
}

UCLASS(BlueprintType, EditInlineNew, DisplayName = "트레이스" )
class MNYS_API UYSAbilityTriggerPayload_Trace : public UYSAbilityTriggerPayload
{
	GENERATED_BODY()
public :
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="YS|Trace", meta = (DisplayName = "트레이스 판정 설정."))
	FYSTraceConfig TraceConfig;
};

// 사실상 AbiityTask에 필요한 파라미터 모두 세팅.
UCLASS(BlueprintType, EditInlineNew, DisplayName = "속도 적용")
class MNYS_API UYSAbilityTriggerPayload_Velocity : public UYSAbilityTriggerPayload
{
	GENERATED_BODY()

public : 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "속도")
	float Velocity = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "시간")
	float Duration = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "방향 정책")
	EYSVelocityDirectionPolicy VelocityDirectionPolicy = EYSVelocityDirectionPolicy::UseActorForwardVector;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "방향으로 액터 돌릴지 여부")
	bool bRotateActorToDirection = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "종료 후 Velocity는 어떻게 할 것인가?")
	ERootMotionFinishVelocityMode FinishVelocityMode = ERootMotionFinishVelocityMode::MaintainLastRootMotionVelocity;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "종료 후 적용할 속도", meta = (EditCondition="FinishVelocityMode==ERootMotionFinishVelocityMode::SetVelocity", EditConditionHides))
	FVector FinishSetVelocity = FVector::ZeroVector;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "종료 후 클램핑 속도", meta = (EditCondition="FinishVelocityMode==ERootMotionFinishVelocityMode::ClampVelocity", EditConditionHides))
	float FinishClampVelocity = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "무브먼트 모드 변경 여부")
	bool bSetNewMovementMode = false; 
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "움직이는 동안 무브먼트 모드", meta = (EditCondition="bSetNewMovementMode", EditConditionHides))
	TEnumAsByte<EMovementMode> MoveMode = EMovementMode::MOVE_Walking;
};

// 예를 들어서 역장 1개 폭탄 2개 뿅뿅 스폰하고 싶다면 이걸 활용하면 될 것으로 보임.
// 근데 만약에 여러개 스폰 가능하다고 하면 어떻게 하지..?
UCLASS(BlueprintType, EditInlineNew, DisplayName = "액터 스폰")
class MNYS_API UYSAbilityTriggerPayload_SpawnActor : public UYSAbilityTriggerPayload
{
	GENERATED_BODY()
	
public : 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "스폰 관련 파라미터 세팅")
	TArray<FYSSpawnActorConfig> SpawnActorConfigs;
};