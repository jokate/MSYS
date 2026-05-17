// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
/**
 * 
 */

namespace YSTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(BlockInput);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(AcceptAbilityInput);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_TraceStart);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_TraceEnd);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_OnHit);
	
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Hit_Normal);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Hit_Big);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Hit_Medium);
};