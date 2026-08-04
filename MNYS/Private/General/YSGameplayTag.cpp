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
	UE_DEFINE_GAMEPLAY_TAG(Event_RunEQS, "Event.RunEQS");
	
	UE_DEFINE_GAMEPLAY_TAG(Event_OnDead, "Event.OnDead");
	UE_DEFINE_GAMEPLAY_TAG(Event_DeathComplete, "Event.DeathComplete");
	UE_DEFINE_GAMEPLAY_TAG(Event_JustAvoid, "Event.JustAvoid");
	UE_DEFINE_GAMEPLAY_TAG(Event_Attack_Activate, "Event.Attack.Activate");
	UE_DEFINE_GAMEPLAY_TAG(Event_TagEnter, "Event.TagEnter");
	UE_DEFINE_GAMEPLAY_TAG(Event_PushCamera, "Event.PushCamera");
	UE_DEFINE_GAMEPLAY_TAG(Event_PopCamera, "Event.PopCamera");
	UE_DEFINE_GAMEPLAY_TAG(Event_AimStart, "Event.AimStart");
	UE_DEFINE_GAMEPLAY_TAG(Event_AimStop, "Event.AimStop");
	
	UE_DEFINE_GAMEPLAY_TAG(Hit_Normal, "Hit.Normal");
	UE_DEFINE_GAMEPLAY_TAG(Hit_Big, "Hit.Big");
	UE_DEFINE_GAMEPLAY_TAG(Hit_Medium, "Hit.Medium");
	
	UE_DEFINE_GAMEPLAY_TAG(JustAvoid_Window, "JustAvoid.Window");
	UE_DEFINE_GAMEPLAY_TAG(Invincible, "Invincible");
	
	
	UE_DEFINE_GAMEPLAY_TAG(Buff_JustAvoid, "Buff.JustAvoid");

	UE_DEFINE_GAMEPLAY_TAG(Input_Phase_Pressed, "Input.Phase.Pressed");
	UE_DEFINE_GAMEPLAY_TAG(Input_Phase_Released, "Input.Phase.Released");

	UE_DEFINE_GAMEPLAY_TAG(Data_Stat_HP,  "Data.Stat.HP");
	UE_DEFINE_GAMEPLAY_TAG(Data_Stat_MEL, "Data.Stat.MEL");
	UE_DEFINE_GAMEPLAY_TAG(Data_Stat_RNG, "Data.Stat.RNG");
	UE_DEFINE_GAMEPLAY_TAG(Data_Stat_AGI, "Data.Stat.AGI");
	UE_DEFINE_GAMEPLAY_TAG(Data_Stat_SYN, "Data.Stat.SYN");
	UE_DEFINE_GAMEPLAY_TAG(Data_Stat_SCL, "Data.Stat.SCL");

	UE_DEFINE_GAMEPLAY_TAG(AISuppress, "AI.Suppress");
	UE_DEFINE_GAMEPLAY_TAG(AISuppress_HitReact, "AI.Suppress.HitReact");
	UE_DEFINE_GAMEPLAY_TAG(AISuppress_Skill, "AI.Suppress.Skill");

	UE_DEFINE_GAMEPLAY_TAG(World_AISuppress, "World.AI.Suppress");
	UE_DEFINE_GAMEPLAY_TAG(World_AISuppress_Cinematic, "World.AI.Suppress.Cinematic");
}