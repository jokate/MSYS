// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "YSGE_SwitchEntryGroggy.generated.h"

/** 스위치 어택 진입기의 그로기 피해. 양은 FYSSwitchEntryForm 이 SetByCaller(Data.Groggy.SwitchEntryDamage)로 넣는다 (애셋 불필요). */
UCLASS()
class MNYS_API UYSGE_SwitchEntryGroggy : public UGameplayEffect
{
	GENERATED_BODY()

public :
	UYSGE_SwitchEntryGroggy();
};
