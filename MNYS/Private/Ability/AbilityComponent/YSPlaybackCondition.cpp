// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/AbilityComponent/YSPlaybackCondition.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Ability/YSGameplayAbility.h"
#include "Input/StateMachine/YSInputStateMachineComponent.h"

bool FYSPlaybackCondition_ContextTag::Evaluate(const TSharedPtr<FYSPlaybackContext>& Context) const
{
	bool bResult = (OperatorType == EYSOperatorType::AND);
	for (const FGameplayTag& RequiredTag : RequiredTags)
	{
		bool bFound = Context->ContextTags.HasTagExact(RequiredTag);
		
		switch (OperatorType)
		{
			case EYSOperatorType::AND:
				{
					bResult &= bFound;
					break;	
				}
			case EYSOperatorType::OR:
				{
					bResult |= bFound;
					break;			
				}	
		}
		
	}
	return bInvert ? !bResult : bResult;
}

bool FYSPlaybackCondition_HasGameplayEffect::Evaluate(const TSharedPtr<FYSPlaybackContext>& Context) const
{
	UYSGameplayAbility* Ability = Context->OwnerAbility;
	
	if (IsValid(Ability) == false)
	{
		return false;
	}
	
	UAbilitySystemComponent* ASC =  Ability->GetAbilitySystemComponentFromActorInfo();
	if ( IsValid(ASC) == false )
	{
		return false;
	}
	
	// 만약 존재한다면 이게 있겄지야.
	return ASC->GetGameplayEffectCount(GameplayEffect, nullptr) > 0;
}

bool FYSPlaybackCondition_ActorInArc::Evaluate(const TSharedPtr<FYSPlaybackContext>& Context) const
{	
	AActor* Target = Context->Target;
	AActor* Instigator = Context->Instigator;
	if (IsValid(Target) == false || IsValid(Instigator) == false)
	{
		return false;
	}
	
	
	const FVector DirectionToTarget = (Target->GetActorLocation() -Instigator->GetActorLocation()).GetSafeNormal();
	const FVector ForwardDirection = Instigator->GetActorForwardVector();
	
	const float DotProduct = FVector::DotProduct(ForwardDirection, DirectionToTarget);
	const float AngleInDegrees = FMath::RadiansToDegrees(FMath::Acos(DotProduct));
	
	return AngleInDegrees <= ArcAngle * 0.5f;
}

bool FYSPlaybackCondition_ActorInDistance::Evaluate(const TSharedPtr<FYSPlaybackContext>& Context) const
{
	AActor* Target = Context->Target;
	AActor* Instigator = Context->Instigator;
	if (IsValid(Target) == false || IsValid(Instigator) == false)
	{
		return false;
	}
	
	float Distance = FVector::Dist(Target->GetActorLocation(), Instigator->GetActorLocation());
	return bIsInRange ? Distance <= CheckDistance : Distance > CheckDistance;
}

bool FYSPlaybackCondition_Input::Evaluate(const TSharedPtr<FYSPlaybackContext>& Context) const
{
	UYSInputStateMachineComponent* InputStateMachine = UYSInputStateMachineComponent::Get(Context->Instigator);
	if (IsValid(InputStateMachine) == false)
	{
		return false;
	}

	return InputStateMachine->ContainsInputHistory(InputHistory.InputTag, InputHistory.InputPhase) ^ bInvert;
}

void FYSPlaybackCondition_Input::OnConditionEvaluatedComplete(const TSharedPtr<FYSPlaybackContext>& Context) const
{
	UYSInputStateMachineComponent* InputStateMachine = UYSInputStateMachineComponent::Get(Context->Instigator);
	if (IsValid(InputStateMachine) == false)
	{
		return;
	}

	InputStateMachine->ConsumeInputHistory(InputHistory.InputTag, InputHistory.InputPhase);
}

bool FYSPlaybackCondition_InputHeld::Evaluate(const TSharedPtr<FYSPlaybackContext>& Context) const
{
	if (Context.IsValid() == false)
	{
		return bInvert;
	}

	const UYSInputStateMachineComponent* InputStateMachine = UYSInputStateMachineComponent::Get(Context->Instigator);

	if (IsValid(InputStateMachine) == false)
	{
		return bInvert;
	}

	return InputStateMachine->IsInputHeld(InputTag) ^ bInvert;
}

bool FYSPlaybackCondition_HasResource::Evaluate(const TSharedPtr<FYSPlaybackContext>& Context) const
{
	UYSGameplayAbility* Ability = Context->OwnerAbility;
	
	if (IsValid(Ability) == false)
	{
		return false;
	}
	
	UAbilitySystemComponent* ASC =  Ability->GetAbilitySystemComponentFromActorInfo();
	if ( IsValid(ASC) == false )
	{
		return false;
	}
	
	return ASC->GetNumericAttribute(ResourceAttribute) >= RequiredAmount;
}
