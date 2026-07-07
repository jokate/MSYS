// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/AIAbility/YSGameplayAbility_AIBase.h"

#include "Ability/AIAbility/YSAIAbilityScoreFunction.h"
#include "AI/YSAIController.h"
#include "AI/Component/YSAIPerceptionComponent.h"
#include "General/YSStruct.h"

float UYSGameplayAbility_AIBase::GetAbilityUtilityScore(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo) const
{
	AActor* OwnerActor = ActorInfo->OwnerActor.Get();
	
	if (IsValid(OwnerActor) == false)
	{
		return 0.f;
	}
	
	const TSharedPtr<FYSTargetingActorCollections> OwnerTargetingActorCollections = UYSAIPerceptionComponent::GetTargetingCollection(OwnerActor);
	
	float TempScore = BaseUtilityScore;
	for (const UYSAIAbilityScoreFunctionBase* Function : UtilityScore )
	{
		TempScore *= Function->GetScoreFactor(ActorInfo, OwnerTargetingActorCollections.Get());
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

bool UYSGameplayAbility_AIBase::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if ( GetAbilityUtilityScore(Handle, ActorInfo) <= UtilityScoreThreshold )
	{
		return false;
	}
	
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}
