// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/AbilityComponent/YSPlaybackAction.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "YSAbilitySystemComponent.h"
#include "Ability/YSGameplayAbility.h"
#include "Ability/Payload/YSAbilityTriggerPayload.h"

void UYSPlaybackAction_SendGameplayEvent::Execute(const TSharedPtr<FYSPlaybackContext>& Context) const
{
	if ( Context.IsValid() == false || IsValid(Context->OwnerAbility) == false || EventData.IsValid() == false )
	{
		return;
	}

	UAbilitySystemComponent* ASC = Context->OwnerAbility->GetAbilitySystemComponentFromActorInfo();

	if ( IsValid(ASC) == false )
	{
		return;
	}

	FGameplayEventData Payload;
	Payload.EventTag = EventData.TargetToTrigger;
	Payload.Instigator = Context->Instigator;
	Payload.Target = Context->Target;

	// AnimNotify 가 쏘는 것과 같은 경로다. 페이로드 슬롯은 두 개뿐이다.
	const TArray<UYSAbilityTriggerPayload*>& TriggerPayloads = EventData.TriggerPayloads;

	if ( TriggerPayloads.IsValidIndex(0) )
	{
		Payload.OptionalObject = TriggerPayloads[0];
	}

	if ( TriggerPayloads.IsValidIndex(1) )
	{
		Payload.OptionalObject2 = TriggerPayloads[1];
	}

	ASC->HandleGameplayEvent(EventData.TargetToTrigger, &Payload);
}

void UYSPlaybackAction_FirstHitTarget::Execute(const TSharedPtr<FYSPlaybackContext>& Context) const
{
	if ( Context.IsValid() == false || IsValid(Context->OwnerAbility) == false )
	{
		return;
	}

	const TSharedPtr<FYSAbilityHitContext>& HitContext = Context->OwnerAbility->GetHitContext();

	if ( HitContext.IsValid() == false )
	{
		return;
	}

	const TArray<AActor*> HitActors = HitContext->GetAllHitActors();

	if ( HitActors.Num() == 0 )
	{
		return;
	}

	Context->Target = HitActors[0];
}

void UYSPlaybackAction_JustAvoidTarget::Execute(const TSharedPtr<FYSPlaybackContext>& Context) const
{
	if ( Context.IsValid() == false )
	{
		return;
	}

	UYSAbilitySystemComponent* ASC = UYSAbilitySystemComponent::Get(Context->Instigator);

	if ( IsValid(ASC) == false )
	{
		return;
	}

	AActor* Attacker = ASC->GetLastJustAvoidInstigator();

	if ( IsValid(Attacker) == false )
	{
		return;
	}

	Context->Target = Attacker;
}

void UYSPlaybackAction_ReleaseBuff::Execute(const TSharedPtr<FYSPlaybackContext>& Context) const
{
	if ( Context.IsValid() == false )
	{
		return;
	}

	if ( UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Context->Instigator) )
	{
		ASC->RemoveActiveEffectsWithTags(BuffTags);
	}
}