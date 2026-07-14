// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "YSEQC_Target.generated.h"

/**
 * 현재 BestTarget 기준으로 Context를 잡을 것임.
 */
UCLASS()
class MNYS_API UYSEQC_Target : public UEnvQueryContext
{
	GENERATED_BODY()
	
public : 
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
};
