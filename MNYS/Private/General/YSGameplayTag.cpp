// Fill out your copyright notice in the Description page of Project Settings.


#include "General/YSGameplayTag.h"


namespace YSTags
{
	UE_DEFINE_GAMEPLAY_TAG(BlockInput, "Block.Input");
	UE_DEFINE_GAMEPLAY_TAG(BlockLockOn, "Block.LockOn");
	UE_DEFINE_GAMEPLAY_TAG(AcceptAbilityInput, "Accept.AbilityInput");
	
	UE_DEFINE_GAMEPLAY_TAG(Event_TraceStart, "Event.TraceStart");
	UE_DEFINE_GAMEPLAY_TAG(Event_TraceEnd, "Event.TraceEnd");
	UE_DEFINE_GAMEPLAY_TAG(Event_OnHit, "Event.OnHit");
	UE_DEFINE_GAMEPLAY_TAG(Event_ApplyVelocity, "Event.ApplyVelocity");
	UE_DEFINE_GAMEPLAY_TAG(Event_SpawnActor, "Event.SpawnActor");
	
	UE_DEFINE_GAMEPLAY_TAG(Event_OnDead, "Event.OnDead");
	UE_DEFINE_GAMEPLAY_TAG(Event_DeathComplete, "Event.DeathComplete");
	UE_DEFINE_GAMEPLAY_TAG(Event_JustAvoid, "Event.JustAvoid");
	UE_DEFINE_GAMEPLAY_TAG(Event_Attack_Activate, "Event.Attack.Activate");
	
	UE_DEFINE_GAMEPLAY_TAG(Hit_Normal, "Hit.Normal");
	UE_DEFINE_GAMEPLAY_TAG(Hit_Big, "Hit.Big");
	UE_DEFINE_GAMEPLAY_TAG(Hit_Medium, "Hit.Medium");
	
	UE_DEFINE_GAMEPLAY_TAG(JustAvoid_Window, "JustAvoid.Window");
	UE_DEFINE_GAMEPLAY_TAG(Invincible, "Invincible");
}