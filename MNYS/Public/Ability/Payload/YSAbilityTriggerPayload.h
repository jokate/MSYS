// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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
	UPROPERTY()
	TWeakObjectPtr<AActor> OwnerActor;
};


UCLASS(BlueprintType, EditInlineNew, DisplayName = "트레이스" )
class MNYS_API UYSAbilityTriggerPayload_Trace : public UYSAbilityTriggerPayload
{
	GENERATED_BODY()

public : 
	static const UYSAbilityTriggerPayload_Trace* GetTracePayload(const FGameplayEventData* EventData);
public :
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="YS|Trace", meta = (DisplayName = "트레이스 판정 설정."))
	FYSTraceConfig TraceConfig;
};