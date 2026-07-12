// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/AIAbility/YSAIAbilityScoreFunction.h"

#include "YSBattleActor.h"
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
	return ProcessIAUSFunction(NormalizedInput);
}

float UYSAIAbilityScoreFunctionBase::ProcessIAUSFunction(float NormalizedInput) const
{
	float RetVal = NormalizedInput;
	switch (IAUSType)
	{
	case EYSIAUSType::LinearQuadratic:
		{
			RetVal = (Slope * FMath::Pow(NormalizedInput - HorizontalShift, Exponential)) + VerticalShift;
			break;
		}
	case EYSIAUSType::Logistic:
		{
			// (1000e)^(-x+c) : 밑이 1000e여야 0~1 입력에서 S-커브가 나온다 (1000·e^(-x+c)는 전 구간 ≈0)
			RetVal =  Exponential * (1.0f / (1.0f + FMath::Pow(1000.0f * UE_EULERS_NUMBER, -NormalizedInput + HorizontalShift))) + VerticalShift;
			break;
		}
	case EYSIAUSType::Logit:
		RetVal =  FMath::Loge((NormalizedInput + UE_EULERS_NUMBER) / (1.0f - NormalizedInput + UE_EULERS_NUMBER)) / Slope + VerticalShift;
		break;
	case EYSIAUSType::Gaussian:
		RetVal =  FMath::Pow(UE_EULERS_NUMBER / (Slope * FMath::Sqrt(2.f * UE_PI)), -0.5f * FMath::Square((NormalizedInput - HorizontalShift) / Slope)) * Exponential + VerticalShift;
		break;
	default:
		return NormalizedInput;
	}
	
	return FMath::Clamp(RetVal, 0.f, 1.f)
;}

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

bool UYSAIAbilityScoreFunction_Hp::GetInputValue(const FGameplayAbilityActorInfo* ActorInfo,
	const FYSTargetingActorCollections* TargetingActorCollections, float& OutRawInput) const
{
	if (ActorInfo == nullptr || TargetingActorCollections == nullptr)
	{
		return false;
	}

	// 내꺼면 내꺼 가져오기
	if ( Subject == EYSScoreObjectType::Owner )
	{
		IYSBattleActor* BattleActor = Cast<IYSBattleActor>(ActorInfo->OwnerActor);
	
		if ( BattleActor == nullptr )
		{
			return false;
		}	
		
		OutRawInput = BattleActor->GetHpRatio();
		return true;
	}
 	
	AActor* BestTargetActor = TargetingActorCollections->GetBestTargetActor();

	if (IsValid(BestTargetActor) == false)
	{
		return false;
	}
	
	IYSBattleActor* TargetBattleActor = Cast<IYSBattleActor>(BestTargetActor);
	if (TargetBattleActor == nullptr)
	{
		return false;
	}

	OutRawInput = TargetBattleActor->GetHpRatio();
	return true;
}
