// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/Task/YSAT_RunEQSQuery.h"

#include "Ability/YSGameplayAbility.h"
#include "AI/YSAIController.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "EnvironmentQuery/EnvQueryManager.h"

UYSAT_RunEQSQuery* UYSAT_RunEQSQuery::CreateTask(UGameplayAbility* OwningAbility, UEnvQuery* QueryTemplate)
{
	UYSAT_RunEQSQuery* MyObj = NewAbilityTask<UYSAT_RunEQSQuery>(OwningAbility);
	MyObj->QueryToActive = QueryTemplate;
	return MyObj;
}

void UYSAT_RunEQSQuery::Activate()
{
	Super::Activate();
	
	UYSGameplayAbility* YSAbility = Cast<UYSGameplayAbility>(GetTaskOwner());
	UEnvQueryManager* EQS = UEnvQueryManager::GetCurrent(this);
	
	if (IsValid(EQS) == false || IsValid(YSAbility) == false )
	{
		EndTask();
		return;
	}
	
	AActor* OwnerActor = YSAbility->GetCurrentActorInfo()->OwnerActor.Get();
	
	if ( IsValid(OwnerActor) == false )
	{
		EndTask();
		return;
	}
	
	AYSAIController* AIController = AYSAIController::Get(OwnerActor);
	if ( IsValid(AIController) == false )
	{
		EndTask();
		return;
	}
	
	TSharedPtr<FEnvQueryResult> QueryResult = EQS->RunInstantQuery(FEnvQueryRequest(QueryToActive, AIController), EEnvQueryRunMode::AllMatching);
	
	if (QueryResult.IsValid() == false || QueryResult->IsAborted() || QueryResult->Items.Num() == 0 )
	{
		UE_LOG(LogTemp, Log, TEXT("EQS Aborted"));
		EndTask();
		return;
	}
	
	// 쿼리 즉시 실행 후 처리.
	const FVector& Location = QueryResult->GetItemAsLocation(0);
	OnSucceeded.Execute(YSAbility, Location);
	EndTask();
}

void UYSAT_RunEQSQuery::OnDestroy(bool bInOwnerFinished)
{
	Super::OnDestroy(bInOwnerFinished);
	
	OnSucceeded.Unbind();
}
