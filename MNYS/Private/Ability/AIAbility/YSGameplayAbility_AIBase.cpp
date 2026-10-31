// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/AIAbility/YSGameplayAbility_AIBase.h"

#include "YSAbilitySystemComponent.h"
#include "Ability/AIAbility/YSAIAbilityScoreFunction.h"
#include "AI/Component/YSAIPerceptionComponent.h"
#include "Data/YSAbilityDataAsset.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "General/YSStruct.h"

bool FYSAIAbilityScore::IsCandidate() const
{
	return IsValid(Ability) && bIsActive == false && bCanActivate && Score > Ability->UtilityScoreThreshold;
}

FYSAIAbilitySelection UYSGameplayAbility_AIBase::SelectAbility(UYSAbilitySystemComponent* ASC)
{
	FYSAIAbilitySelection Selection;

	if ( IsValid(ASC) == false )
	{
		return Selection;
	}

	const UYSAbilityDataAsset* AbilityDataAsset = UYSAbilityDataAsset::GetDataAssetFromAbilitySystemComponent(ASC);
	const FGameplayAbilityActorInfo* ActorInfo = ASC->AbilityActorInfo.Get();

	if ( IsValid(AbilityDataAsset) == false || ActorInfo == nullptr )
	{
		return Selection;
	}

	const TSharedPtr<FYSTargetingActorCollections> TargetingCollections = UYSAIPerceptionComponent::GetTargetingCollection(ActorInfo->OwnerActor.Get());

	if ( TargetingCollections.IsValid() == false )
	{
		return Selection;
	}

	for ( const FYSGrantedAbilityData& GrantedAbilityData : AbilityDataAsset->GetAllAbilities() )
	{
		const FGameplayAbilitySpec* AbilitySpec = ASC->FindAbilitySpecFromClass(GrantedAbilityData.AbilityClass);

		if ( AbilitySpec == nullptr )
		{
			continue;
		}

		// NonInstanced 어빌리티는 인스턴스가 없으므로 CDO로 폴백
		const UGameplayAbility* AbilityObject = AbilitySpec->GetPrimaryInstance();
		if ( IsValid(AbilityObject) == false )
		{
			AbilityObject = AbilitySpec->Ability;
		}

		const UYSGameplayAbility_AIBase* AIAbility = Cast<UYSGameplayAbility_AIBase>(AbilityObject);

		if ( IsValid(AIAbility) == false )
		{
			continue;
		}

		const int32 Index = Selection.Scores.Add(AIAbility->EvaluateScore(*AbilitySpec, ActorInfo, TargetingCollections.Get()));
		const FYSAIAbilityScore& Entry = Selection.Scores[Index];

		// 동률이면 데이터 애셋 앞쪽이 남는다.
		const FYSAIAbilityScore* Best = Selection.GetBest();
		if ( Entry.IsCandidate() && (Best == nullptr || Entry.Score > Best->Score) )
		{
			Selection.BestIndex = Index;
		}
	}

	return Selection;
}

FYSAIAbilityScore UYSGameplayAbility_AIBase::EvaluateScore(const FGameplayAbilitySpec& Spec,
	const FGameplayAbilityActorInfo* ActorInfo, const FYSTargetingActorCollections* TargetingCollections) const
{
	FYSAIAbilityScore Result;
	Result.Ability = this;
	Result.Handle = Spec.Handle;
	Result.bIsActive = Spec.IsActive();
	Result.bCanActivate = CanActivateAbility(Spec.Handle, ActorInfo);

	for ( const UYSAIAbilityScoreFunctionBase* Function : UtilityScore )
	{
		if ( IsValid(Function) == false )
		{
			continue;
		}

		const float Value = Function->GetScoreFactor(ActorInfo, TargetingCollections);
		Result.Factors.Emplace(Function, Value);
		Result.Factor *= Value;
	}

	Result.Score = BaseUtilityScore * Result.Factor;
	return Result;
}

void UYSGameplayAbility_AIBase::SetupPlayBack(const FGameplayEventData* TriggerEventData)
{
	const TSharedPtr<FYSTargetingActorCollections> TargetingCollections = UYSAIPerceptionComponent::GetTargetingCollection(GetOwningActorFromActorInfo());

	if ( TargetingCollections.IsValid() == false )
	{
		return;
	}

	// 노드는 Super 가 그래프에서 고른다. 재생이 Super 안에서 시작되므로 타겟을 먼저 넣는다.
	PlaybackContext->Target = TargetingCollections->GetBestTargetActor();

	Super::SetupPlayBack(TriggerEventData);
}

UEnvQuery* UYSGameplayAbility_AIBase::GetQueryToReposition() const
{
	// IsValid() 는 이미 로드된 경우만 참이라, 비었는지는 IsNull() 로 본다.
	if ( QueryToReposition.IsNull() )
	{
		return nullptr;
	}

	return QueryToReposition.LoadSynchronous();
}
