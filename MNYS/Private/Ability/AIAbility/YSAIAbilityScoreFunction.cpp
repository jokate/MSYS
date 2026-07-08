// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/AIAbility/YSAIAbilityScoreFunction.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "General/YSStruct.h"

float UYSAIAbilityScoreFunctionBase::GetScoreFactor(const FGameplayAbilityActorInfo* ActorInfo,
	const FYSTargetingActorCollections* TargetingActorCollections) const
{
	float RawInput = 0.f;
	if (GetInputValue(ActorInfo, TargetingActorCollections, RawInput) == false)
	{
		return 0.f;
	}

	const float RangeSize = FMath::Max(InputRange.Size(), KINDA_SMALL_NUMBER);
	const float NormalizedInput = FMath::Clamp((RawInput - InputRange.Min) / RangeSize, 0.f, 1.f);

	const FRichCurve* Curve = ResponseCurve.GetRichCurveConst();
	if (Curve != nullptr && Curve->GetNumKeys() > 0)
	{
		return FMath::Clamp(Curve->Eval(NormalizedInput), 0.f, 1.f);
	}

	return NormalizedInput;
}

bool UYSAIAbilityScoreFunction_Distance::GetInputValue(const FGameplayAbilityActorInfo* ActorInfo,
	const FYSTargetingActorCollections* TargetingActorCollections, float& OutRawInput) const
{
	if (ActorInfo == nullptr || TargetingActorCollections == nullptr)
	{
		return false;
	}

	AActor* OwnerActor = ActorInfo->OwnerActor.Get();
	AActor* BestTargetActor = TargetingActorCollections->GetBestTargetActor();

	if (IsValid(OwnerActor) == false || IsValid(BestTargetActor) == false)
	{
		return false;
	}

	OutRawInput = FVector::Dist(OwnerActor->GetActorLocation(), BestTargetActor->GetActorLocation());
	return true;
}
