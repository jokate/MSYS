// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/AIAbility/YSGameplayAbility_AIBase.h"

#include "Ability/AIAbility/YSAIAbilityScoreFunction.h"
#include "AI/YSAIController.h"
#include "AI/Component/YSAIPerceptionComponent.h"
#include "EnvironmentQuery/EnvQuery.h"
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
	TempScore *= GetUtilityScoreFactor(ActorInfo, OwnerTargetingActorCollections.Get());

	return TempScore;
}

void UYSGameplayAbility_AIBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                const FGameplayEventData* TriggerEventData)
{
		
	TargetingActorCollections = UYSAIPerceptionComponent::GetTargetingCollection(ActorInfo->OwnerActor.Get());
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UYSGameplayAbility_AIBase::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	TargetingActorCollections = nullptr;
}

void UYSGameplayAbility_AIBase::SetupPlayBack(const FGameplayEventData* TriggerEventData)
{
	if (!Playbacks.IsValidIndex(0) || TargetingActorCollections.IsValid() == false )
	{
		return;
	}

	PlaybackContext->OwnerAbility = this;
	PlaybackContext->Instigator = GetOwningActorFromActorInfo();
	PlaybackContext->Target = TargetingActorCollections->GetBestTargetActor();

	ActivePlayback(0);
}

float UYSGameplayAbility_AIBase::GetUtilityScoreFactor(const FGameplayAbilityActorInfo* ActorInfo,
	const FYSTargetingActorCollections* TargetingCollections) const
{
	float Origin = 1.f;
	for (const UYSAIAbilityScoreFunctionBase* Function : UtilityScore )
	{
		if (IsValid(Function) == false)
		{
			continue;
		}

		Origin *= Function->GetScoreFactor(ActorInfo, TargetingCollections);
	}

	return Origin;
}

UEnvQuery* UYSGameplayAbility_AIBase::GetQueryToReposition() const
{
	if ( QueryToReposition.IsValid() == false )
	{
		return nullptr;
	}
	
	return QueryToReposition.IsPending() ? QueryToReposition.LoadSynchronous() : QueryToReposition.Get();
}
