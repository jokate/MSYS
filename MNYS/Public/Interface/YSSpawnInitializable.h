// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "YSSpawnInitializable.generated.h"

struct FYSAbilityHitContext;
UINTERFACE()
class UYSSpawnInitializable : public UInterface
{
	GENERATED_BODY()
};

class MNYS_API IYSSpawnInitializable
{
	GENERATED_BODY()

public:
	virtual bool OnSpawnInitialize(AActor* InOwnerActor, AActor* InInstigator,  const TSharedPtr<FYSAbilityHitContext>& HitContext) = 0;
	
	virtual void SetPoolActive(bool bActive)
	{
		bPoolActive = bActive;
	}
	
	virtual bool IsPoolActive() const 
	{
		return bPoolActive;
	};
	
private :
	bool bPoolActive = true;
};
