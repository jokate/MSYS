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

UYSAIUseAbilityTask* UYSAIUseAbilityTask::CreateTask(AAIController* Controller, const FGameplayAbilitySpecHandle& AbilityHandle)
{
	UYSAIUseAbilityTask* Task = Controller ? UAITask::NewAITask<UYSAIUseAbilityTask>(*Controller, EAITaskPriority::High) : nullptr;
	Task->TargetToActivateAbilityHandle = AbilityHandle;
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
	
	if ( IsValid( OwnerController ) == false || IsValid(OwnerController->GetPawn()) == false )
	{
		EndTask();
		return;
	}

	APawn* OwnerPawn = OwnerController->GetPawn();

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwnerPawn);

	if ( IsValid(ASC) == false )
	{
		EndTask();
		return;
	}
	
	ASC->OnAbilityEnded.AddUObject(this, &ThisClass::OnAbilityEnded);
	
	if ( ASC->TryActivateAbility(TargetToActivateAbilityHandle) == false )
	{
		EndTask();	
	}
}
