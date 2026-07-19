// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/StateTree/Evaluator/YSStateTreeEvaluator.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "StateTreeExecutionContext.h"
#include "AI/YSAIController.h"
#include "AI/Component/YSAIPerceptionComponent.h"
#include "General/YSStruct.h"
#include "StateTreeLinker.h"
#include "YSAbilitySystemComponent.h"
#include "Data/YSAbilityDataAsset.h"
#include "Ability/AIAbility/YSGameplayAbility_AIBase.h"

bool FYSStateTreeEvaluatorTarget::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(AIControllerHandle);
	return true;
}

void FYSStateTreeEvaluatorTarget::TreeStart(FStateTreeExecutionContext& Context) const
{
	const AYSAIController* AIController = Context.GetExternalDataPtr(AIControllerHandle);
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	if (IsValid(AIController) == false || IsValid(AIController->MainPerceptionComponent) == false )
	{
		return;
	}
	
	UYSAIPerceptionComponent* MainPerception = AIController->MainPerceptionComponent;
	
	InstanceData.TargetingActorCollections = MainPerception->GetTargetingActorCollections();
}

void FYSStateTreeEvaluatorTarget::Tick(FStateTreeExecutionContext& Context, float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	if ( InstanceData.TargetingActorCollections == nullptr )
	{
		return;
	}
	
	const TSharedPtr<FYSTargetingActorCollections>& Targeting = InstanceData.TargetingActorCollections;

	if (Targeting.IsValid() == false)
	{
		InstanceData.BestTarget = nullptr;
		InstanceData.SkillTargets.Reset();
		return;
	}
	
	InstanceData.BestTarget = Targeting->GetBestTargetActor();
	
	TArray<AActor*> RawTargets = Targeting->GetSkillTargetActors();
	InstanceData.SkillTargets.Reset(RawTargets.Num());
	
	for (AActor* Target : RawTargets)
	{
		InstanceData.SkillTargets.Add(Target);
	}
}

void FYSStateTreeEvaluatorTarget::TreeStop(FStateTreeExecutionContext& Context) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	
	InstanceData.SkillTargets.Reset();
	InstanceData.BestTarget = nullptr;
	InstanceData.TargetingActorCollections = nullptr;
}

void FYSStateTreeEvaluatorAbilityActivation::TreeStart(FStateTreeExecutionContext& Context) const
{
	const AYSAIController* AIController = Context.GetExternalDataPtr(AIControllerHandle);
	
	if ( IsValid(AIController) == false )
	{
		return;
	}
	
	APawn* TempPawn = AIController->GetPawn();
	
	if ( IsValid(TempPawn) == false )
	{
		return;
	} 
	
	OwnerPawn = TempPawn;
	
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TempPawn);
	
	if ( IsValid(ASC) == false )
	{
		return;
	}
	
	YSASC = Cast<UYSAbilitySystemComponent>(ASC);
}

bool FYSStateTreeEvaluatorAbilityActivation::Link(FStateTreeLinker& Linker)
{	
	Linker.LinkExternalData(AIControllerHandle);
	return true;
}

void FYSStateTreeEvaluatorAbilityActivation::Tick(FStateTreeExecutionContext& Context, float DeltaTime) const
{
	if ( YSASC.IsValid() == false || OwnerPawn.IsValid() == false )
	{
		return;
	}
	
	UYSAbilityDataAsset* AbilityDataAsset = UYSAbilityDataAsset::GetDataAssetFromAbilitySystemComponent(YSASC.Get());

	if ( IsValid(AbilityDataAsset) == false )
	{
		return;
	}

	const TSharedPtr<FYSTargetingActorCollections> TargetingActorCollections = UYSAIPerceptionComponent::GetTargetingCollection(OwnerPawn.Get());
	const FGameplayAbilityActorInfo* ActorInfo = YSASC->AbilityActorInfo.Get();

	if ( TargetingActorCollections.IsValid() == false || ActorInfo == nullptr )
	{
		return;
	}

	float BestScoreFactor = 0.f;
	float BestUtilityScore = 0.f;
	FGameplayAbilitySpecHandle BestAbilityHandle;
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	
	bool bCanActiveAbility = false;
	UEnvQuery* QueryToReposition = nullptr;
	for ( const FYSGrantedAbilityData& GrantedAbilityData : AbilityDataAsset->GetAllAbilities() )
	{
		const FGameplayAbilitySpec* AbilitySpec = YSASC->FindAbilitySpecFromClass(GrantedAbilityData.AbilityClass);

		if ( AbilitySpec == nullptr || AbilitySpec->IsActive() )
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
		
		// 가중치 존재 값
		const float UtilityScore = AIAbility->GetAbilityUtilityScore(AbilitySpec->Handle, ActorInfo);
		// 가중치가 없는 값.
		const float UtilityScoreFactor = AIAbility->GetUtilityScoreFactor(ActorInfo, TargetingActorCollections.Get());
		
		// 사용 여부
		bool bCurrentlyActivatable =  AIAbility->CanActivateAbility(AbilitySpec->Handle, ActorInfo) && UtilityScore > AIAbility->UtilityScoreThreshold;
		
		// 확률이 같은 경우
		if ( bCurrentlyActivatable == false )
		{
			continue;
		}
		
		bCanActiveAbility = true;
		if ( UtilityScoreFactor > BestScoreFactor )
		{
			BestUtilityScore = UtilityScoreFactor;
			BestScoreFactor = UtilityScoreFactor;
			BestAbilityHandle = AbilitySpec->Handle;
			QueryToReposition = AIAbility->GetQueryToReposition();
			s
			UE_LOG(LogTemp, Log, TEXT("Best UtilityScore: %f / %s"), UtilityScoreFactor, *AIAbility->GetName());
			
		}
		// 가중치에 의해서 동작.s
		else if ( UtilityScoreFactor == BestScoreFactor)
		{
			if ( UtilityScore > BestUtilityScore )
			{
				BestUtilityScore = UtilityScore;
				BestAbilityHandle = AbilitySpec->Handle;
			}
		}
	}

	InstanceData.bIsEnableActivateAbility = bCanActiveAbility;
	InstanceData.BestAbilitySpecHandle = BestAbilityHandle;
	InstanceData.EnvQueryInstance = QueryToReposition;
}