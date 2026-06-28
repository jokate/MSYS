// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "General/YSEnum.h"
#include "YSEnvQueryTest_TeamCheck.generated.h"

/**
 * 
 */
UCLASS(DisplayName = "팀 체크 테스트")
class MNYS_API UYSEnvQueryTest_TeamCheck : public UEnvQueryTest
{
	GENERATED_BODY()

public :
	UYSEnvQueryTest_TeamCheck();
	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;

	// EQS 에디터 노드에 표시될 제목/상세
	virtual FText GetDescriptionTitle() const override;
	virtual FText GetDescriptionDetails() const override;

public : 
	UPROPERTY(EditDefaultsOnly, Category="YS | Context")
	TSubclassOf<UEnvQueryContext> ContextCheck;
	
	UPROPERTY(EditDefaultsOnly, Category = "YS | Team")
	EYSPerceptionAffiliation Affiliation = EYSPerceptionAffiliation::Any;
};
