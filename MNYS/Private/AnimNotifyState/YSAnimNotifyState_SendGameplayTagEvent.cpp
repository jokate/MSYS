// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState/YSAnimNotifyState_SendGameplayTagEvent.h"
#include "Ability/Payload/YSAbilityTriggerPayload.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbilityTypes.h"

void UYSAnimNotifyState_SendGameplayTagEvent::NotifyBegin(USkeletalMeshComponent* MeshComp,
                                                          UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	SendGameplayEvent(MeshComp, TriggerGameplayData);
}

void UYSAnimNotifyState_SendGameplayTagEvent::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	SendGameplayEvent(MeshComp, EndTriggerGameplayData);
}

void UYSAnimNotifyState_SendGameplayTagEvent::SendGameplayEvent(USkeletalMeshComponent* MeshComp,
	const FGameplayEventSendData& NotifyData)
{
	AActor* OwnerActor = MeshComp->GetOwner();

	if ( IsValid(OwnerActor) == false )
	{
		return;
	} 
	
	FGameplayEventData EventData;

	const TArray<UYSAbilityTriggerPayload*>& Payloads = NotifyData.TriggerPayloads;
	for ( int32 i = 0; i < 2; ++i )
	{
		if ( Payloads.IsValidIndex(i) == false )
		{
			continue;
		}

		i == 0 ? EventData.OptionalObject = Payloads[i] : EventData.OptionalObject2 = Payloads[i];
	}

	EventData.Instigator = OwnerActor;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerActor, NotifyData.TargetToTrigger, EventData);
}
