// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "YSBattleActor.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UYSBattleActor : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MNYS_API IYSBattleActor
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual bool IsDead() const { return false; }

};
