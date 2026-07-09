// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/AITask/YSAIUseAbilityTask.h"

#include "AbilitySystemGlobals.h"
#include "AIController.h"
#include "YSAbilitySystemComponent.h"
#include "Ability/AIAbility/YSGameplayAbility_AIBase.h"
#include "AI/Component/YSAIPerceptionComponent.h"
#include "Data/YSAbilityDataAsset.h"
#include "General/YSStruct.h"

UYSAIUseAbilityTask::UYSAIUseAbilityTask(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bTickingTask = true;
}

UYSAIUseAbilityTask* UYSAIUseAbilityTask::CreateTask(AAIController* Controller)
{
	UYSAIUseAbilityTask* Task = Controller ? UAITask::NewAITask<UYSAIUseAbilityTask>(*Controller, EAITaskPriority::High) : nullptr;
	
	return Task;
}

void UYSAIUseAbilityTask::OnAbilityEnded(const FAbilityEndedData& AbilityEndedData)
{
	if ( IsValid(OwnerController) == false )
	{
		return;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwnerController->GetPawn());

	if ( IsValid(ASC) == false )
	{
		return;
	}

	ASC->OnAbilityEnded.RemoveAll(this);
	EndTask();
}

void UYSAIUseAbilityTask::Activate()
{
	Super::Activate();
}

void UYSAIUseAbilityTask::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);
	
	if ( bIsActiveAbil == false )
	{
		_TryActivateAbility();
	}
}

void UYSAIUseAbilityTask::_TryActivateAbility()
{
		if ( IsValid( OwnerController ) == false || IsValid(OwnerController->GetPawn()) == false )
	{
		EndTask();
		return;
	}

	APawn* OwnerPawn = OwnerController->GetPawn();

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwnerPawn);

	if ( IsValid(ASC) == false )
	{
		return;
	}

	UYSAbilitySystemComponent* YSASC = Cast<UYSAbilitySystemComponent>(ASC);

	if ( IsValid(YSASC) == false )
	{
		return;
	}

	UYSAbilityDataAsset* AbilityDataAsset = UYSAbilityDataAsset::GetDataAssetFromAbilitySystemComponent(YSASC);

	if ( IsValid(AbilityDataAsset) == false )
	{
		return;
	}

	const TSharedPtr<FYSTargetingActorCollections> TargetingActorCollections = UYSAIPerceptionComponent::GetTargetingCollection(OwnerPawn);
	const FGameplayAbilityActorInfo* ActorInfo = YSASC->AbilityActorInfo.Get();

	if ( TargetingActorCollections.IsValid() == false || ActorInfo == nullptr )
	{
		return;
	}

	float BestScore = 0.f;
	FGameplayAbilitySpecHandle BestAbilityHandle;

	for ( const FYSGrantedAbilityData& GrantedAbilityData : AbilityDataAsset->GetAllAbilities() )
	{
		const FGameplayAbilitySpec* AbilitySpec = YSASC->FindAbilitySpecFromClass(GrantedAbilityData.AbilityClass);

		if ( AbilitySpec == nullptr || AbilitySpec->IsActive() )
		{
			continue;
		}

		// NonInstanced 어빌리티는 인스턴스가 없으므로 CDO로 폴백
		const UGameplayAbility* AbilityObject = AbilitySpec->GetPrimaryInstance();
		if ( AbilityObject == nullptr )
		{
			AbilityObject = AbilitySpec->Ability;
		}

		const UYSGameplayAbility_AIBase* AIAbility = Cast<UYSGameplayAbility_AIBase>(AbilityObject);

		if ( IsValid(AIAbility) == false )
		{
			continue;
		}
		
		if ( AIAbility->CanActivateAbility(AbilitySpec->Handle, ActorInfo) == false )
		{
			continue;
		}
		
		const float UtilityScore = AIAbility->GetAbilityUtilityScore(AbilitySpec->Handle, ActorInfo);

		if ( UtilityScore <= AIAbility->UtilityScoreThreshold )
		{
			continue;
		}

		if ( UtilityScore > BestScore )
		{
			BestScore = UtilityScore;
			BestAbilityHandle = AbilitySpec->Handle;
		}
	}

	if ( BestAbilityHandle.IsValid() == false )
	{
		return;
	}

	YSASC->OnAbilityEnded.AddUObject(this, &ThisClass::OnAbilityEnded);
	YSASC->TryActivateAbility(BestAbilityHandle);
	
	bIsActiveAbil = true;
}
