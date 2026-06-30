// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/StateTree/Condition/YSAIStateTreeConditionInstanceData.h"

#include "StateTreeExecutionContext.h"
#include "AI/YSAIController.h"
#include "AI/Component/YSAIPerceptionComponent.h"


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
