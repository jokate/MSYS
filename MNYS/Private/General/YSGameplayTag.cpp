// Fill out your copyright notice in the Description page of Project Settings.


#include "General/YSGameplayTag.h"


namespace YSTags
{
	UE_DEFINE_GAMEPLAY_TAG(BlockInput, "Block.Input");
	UE_DEFINE_GAMEPLAY_TAG(AcceptAbilityInput, "Accept.AbilityInput");
	
	UE_DEFINE_GAMEPLAY_TAG(Event_TraceStart, "Event.TraceStart");
	UE_DEFINE_GAMEPLAY_TAG(Event_TraceEnd, "Event.TraceEnd");
	
	UE_DEFINE_GAMEPLAY_TAG(Command_UpAttack, "Command.UpAttack");
	UE_DEFINE_GAMEPLAY_TAG(Command_DownAttack, "Command.DownAttack");
	UE_DEFINE_GAMEPLAY_TAG(Command_BackDodgeAttack, "Command.BackDodgeAttack");
}