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
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(BlockLockOn);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(AcceptAbilityInput);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_TraceStart);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_TraceEnd);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_OnHit);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_ApplyVelocity);
	
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Hit_Normal);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Hit_Big);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Hit_Medium);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(JustAvoid_Window);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Invincible);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_OnDead);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_DeathComplete);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_JustAvoid);
};