// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryGenerator.h"
#include "YSEnvQueryGeneratorBase.generated.h"

class AYSMonster;
class AYSAIController;
/**
 * 
 */
UCLASS()
class MNYS_API UYSEnvQueryGeneratorBase : public UEnvQueryGenerator
{
	GENERATED_BODY()
	
public :
	virtual AYSAIController* GetAIControllerFromInstance(FEnvQueryInstance& QueryInstance) const;
	virtual AYSMonster* GetMonsterFromInstance(FEnvQueryInstance& QueryInstance) const;
};
