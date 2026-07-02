// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/AIAbility/YSGameplayAbility_AIBase.h"

#include "Ability/AIAbility/YSAIAbilityScoreFunction.h"
#include "AI/YSAIController.h"
#include "AI/Component/YSAIPerceptionComponent.h"
#include "General/YSStruct.h"

float UYSGameplayAbility_AIBase::GetAbilityUtilityScore(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FYSTargetingActorCollections* InTargetingActorCollections) const
{
	float TempScore = BaseUtilityScore;
	for (const UYSAIAbilityScoreFunctionBase* Function : UtilityScore )
	{
		TempScore += Function->GetScoreFactor(ActorInfo, InTargetingActorCollections);
	}
	
	return TempScore;
}

void UYSGameplayAbility_AIBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	TargetingActorCollections = UYSAIPerceptionComponent::GetTargetingCollection(ActorInfo->OwnerActor.Get());
}

void UYSGameplayAbility_AIBase::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	
	TargetingActorCollections = nullptr;
}
