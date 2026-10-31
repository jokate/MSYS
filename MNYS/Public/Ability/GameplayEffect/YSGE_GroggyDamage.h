// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "YSGE_GroggyDamage.generated.h"

/** 피해 행의 그로기 피해. 양은 UYSGroggyAttributeSet::ApplyGroggyDamage 가 SetByCaller(Data.Groggy.Damage)로 넣는다. */
UCLASS()
class MNYS_API UYSGE_GroggyDamage : public UGameplayEffect
{
	GENERATED_BODY()

public :
	UYSGE_GroggyDamage();
};
