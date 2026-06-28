// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/EQS/YSEnvQueryGeneratorBase.h"

#include "AI/YSAIController.h"
#include "Character/YSMonster.h"

AYSAIController* UYSEnvQueryGeneratorBase::GetAIControllerFromInstance(FEnvQueryInstance& QueryInstance) const
{
	UObject* Object = QueryInstance.Owner.Get();
	
	if ( IsValid(Object) == false )
	{
		return nullptr;
	}
	
	return Cast<AYSAIController>(Object);
}

AYSMonster* UYSEnvQueryGeneratorBase::GetMonsterFromInstance(FEnvQueryInstance& QueryInstance) const
{
	AYSAIController* AIController = GetAIControllerFromInstance(QueryInstance);
	
	if ( IsValid(AIController) == false )
	{
		return nullptr;
	}
	
	return AIController->GetPawn<AYSMonster>();
}
