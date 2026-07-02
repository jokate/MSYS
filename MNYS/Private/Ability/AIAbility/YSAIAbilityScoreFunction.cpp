// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/AIAbility/YSAIAbilityScoreFunction.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "General/YSStruct.h"

float UYSAIAbilityScoreFunction_Distance::GetScoreFactor(const FGameplayAbilityActorInfo* ActorInfo,
                                                         const FYSTargetingActorCollections* TargetingActorCollections) const
{
	AActor* OwnerActor = ActorInfo->OwnerActor.Get();
		
	AActor* BestTargetActor = TargetingActorCollections->GetBestTargetActor();
	if ( IsValid(OwnerActor) == false || IsValid(BestTargetActor) == false )
	{
		return 0.f;
	}
	
	const float CurrentDistance = FVector::Dist(OwnerActor->GetActorLocation(), BestTargetActor->GetActorLocation());
	const float SafeDivisor = bIsRange ? FMath::Max(CurrentDistance, KINDA_SMALL_NUMBER) : FMath::Max(Distance, KINDA_SMALL_NUMBER);
	
	const float RawRatio = bIsRange ? (Distance / SafeDivisor) : (CurrentDistance / SafeDivisor);

	return FMath::Clamp(RawRatio, 0.f, 1.f);
}
