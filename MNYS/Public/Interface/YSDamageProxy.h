// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "YSDamageProxy.generated.h"

UINTERFACE()
class UYSDamageProxy : public UInterface
{
	GENERATED_BODY()
};

class MNYS_API IYSDamageProxy
{
	GENERATED_BODY()

public :
	virtual AActor* GetDamageInstigator() const = 0;
};
