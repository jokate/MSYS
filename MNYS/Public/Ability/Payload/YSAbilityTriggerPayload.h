// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "YSAbilityTriggerPayload.generated.h"

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
