// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
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
