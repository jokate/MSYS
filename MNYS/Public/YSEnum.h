// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "YSEnum.generated.h"

/**
 * 
 */
UENUM()
enum EYSInputStatesType : uint8
{
	None = 0,
	Idle,
	Attack,
	Dodge,
	JustAvoid,
	Falling,
	Skill,
	Ready,
	End,
};
