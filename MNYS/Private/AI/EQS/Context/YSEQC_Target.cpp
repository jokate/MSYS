// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/EQS/Context/YSEQC_Target.h"

#include "AI/YSAIController.h"
#include "AI/Component/YSAIPerceptionComponent.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"


void UYSEQC_Target::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	AYSAIController* QueryOwner = Cast<AYSAIController>(QueryInstance.Owner.Get());
	
	if ( IsValid(QueryOwner) == false )
	{
		return;
	}
	

	AActor* BestTargetActor = QueryOwner->MainPerceptionComponent->GetBestTargetActor();
	
	if ( IsValid(BestTargetActor) == false )
	{
		return;
	}
	
	UEnvQueryItemType_Actor::SetContextHelper(ContextData, BestTargetActor);
}
