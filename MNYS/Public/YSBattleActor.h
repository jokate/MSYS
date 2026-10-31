// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "YSBattleActor.generated.h"

UINTERFACE()
class UYSBattleActor : public UInterface
{
	GENERATED_BODY()
};

class MNYS_API IYSBattleActor
{
	GENERATED_BODY()

public:
	virtual bool IsDead() const { return false; }
	virtual float GetHpRatio() const { return 0.f; }
};
