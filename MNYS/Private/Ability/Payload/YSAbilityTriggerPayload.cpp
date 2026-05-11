// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/Payload/YSAbilityTriggerPayload.h"

#include "Abilities/GameplayAbilityTypes.h"

const UYSAbilityTriggerPayload_Trace* UYSAbilityTriggerPayload_Trace::GetTracePayload(const FGameplayEventData* EventData)
{
	if ( EventData->OptionalObject == nullptr && EventData->OptionalObject2 == nullptr )
		return nullptr;
	
	const UYSAbilityTriggerPayload_Trace* TraceData = Cast<UYSAbilityTriggerPayload_Trace>(EventData->OptionalObject);

	return IsValid(TraceData) ? TraceData : Cast<UYSAbilityTriggerPayload_Trace>(EventData->OptionalObject2);
}
