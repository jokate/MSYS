// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/AbilityComponent/YSPlaybackAction.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "YSAbilitySystemComponent.h"
#include "Ability/YSGameplayAbility.h"
#include "Ability/Payload/YSAbilityTriggerPayload.h"
#include "Character/Components/YSCombatTargetComponent.h"

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

	const FGameplayEventData Payload = EventData.MakeEventData(Context->Instigator, Context->Target);
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

void UYSPlaybackAction_CombatTarget::Execute(const TSharedPtr<FYSPlaybackContext>& Context) const
{
	if ( Context.IsValid() == false )
	{
		return;
	}

	UYSCombatTargetComponent* CombatTarget = UYSCombatTargetComponent::Get(Context->Instigator);
	Context->Target = IsValid(CombatTarget) ? CombatTarget->RefreshTarget() : nullptr;
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