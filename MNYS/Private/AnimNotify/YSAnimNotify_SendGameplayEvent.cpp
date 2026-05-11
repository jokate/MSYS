// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify/YSAnimNotify_SendGameplayEvent.h"
#include "Ability/Payload/YSAbilityTriggerPayload.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbilityTypes.h"

void UYSAnimNotify_SendGameplayEvent::SendGameplayEvent(USkeletalMeshComponent* MeshComp,
                                                        const FGameplayEventSendData& InTriggerGameplayData)
{
	AActor* OwnerActor = MeshComp->GetOwner();

	if ( IsValid(OwnerActor) == false )
	{
		return;
	} 
	
	FGameplayEventData EventData;

	const TArray<UYSAbilityTriggerPayload*>& Payloads = InTriggerGameplayData.TriggerPayloads;
	for ( int32 i = 0; i < 2; ++i )
	{
		if ( Payloads.IsValidIndex(i) == false )
		{
			continue;
		}

		i == 0 ? EventData.OptionalObject = Payloads[i] : EventData.OptionalObject2 = Payloads[i];
	}

	EventData.Instigator = OwnerActor;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerActor, InTriggerGameplayData.TargetToTrigger, EventData);
}

void UYSAnimNotify_SendGameplayEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                             const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	SendGameplayEvent(MeshComp, TriggerGameplayData);
}
