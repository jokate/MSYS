// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/StateTree/Condition/YSAIStateTreeConditionInstanceData.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "StateTreeExecutionContext.h"
#include "AI/YSAIController.h"
#include "AI/Component/YSAIPerceptionComponent.h"
#include "Subsystem/YSWorldTagSubsystem.h"


AYSAIController* FYSAIStateTreeConditionBase::GetAIControllerFromContext(FStateTreeExecutionContext& Context) const
{
	UObject* Object = Context.GetOwner();
	if ( IsValid(Object) == false )
	{
		return nullptr;
	}
	
	return Cast<AYSAIController>(Object);
}

bool FYSTargetingActorCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	UInstanceDataType* InstanceData = Context.GetInstanceDataPtr<UInstanceDataType>(*this);
	
	if ( InstanceData == nullptr ) 
		return false;
	
	AYSAIController* Controller = GetAIControllerFromContext(Context);
	
	if ( IsValid(Controller) == false )
	{
		return false;
	}
	
	UYSAIPerceptionComponent* AIPerceptionComponent = Controller->MainPerceptionComponent;
	
	if ( IsValid(AIPerceptionComponent) == false )
	{
		return false;
	}
	
	return IsValid(AIPerceptionComponent->GetBestTargetActor()) ^ InstanceData->bInvert;
}

bool FYSHasTagCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	UInstanceDataType* InstanceData = Context.GetInstanceDataPtr<UInstanceDataType>(*this);
	
	if ( InstanceData == nullptr ) 
		return false;
	
	AYSAIController* Controller = GetAIControllerFromContext(Context);
	
	if ( IsValid(Controller) == false )
	{
		return false;
	}
	
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Controller->GetPawn());
	
	if ( IsValid(ASC) == false )
	{
		return false;
	}
	
	bool bRetVal = ASC->HasMatchingGameplayTag(InstanceData->ConsiderationTag);
	
	if ( InstanceData->bConsiderationWorld )
	{
		UWorld* World = Context.GetWorld();
		
		UYSWorldTagSubsystem* TagSubsystem = World->GetSubsystem<UYSWorldTagSubsystem>();
		
		if ( IsValid(TagSubsystem) == false )
		{
			return false;
		}
		
		bRetVal |= TagSubsystem->HasWorldTagMatching(InstanceData->ConsiderationTag);
	}
	
	return bRetVal;
}
