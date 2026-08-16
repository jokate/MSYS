// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "YSPlaybackGraphFactory.generated.h"

/** 콘텐츠 브라우저 우클릭 → 플레이백 그래프 생성. */
UCLASS()
class UYSPlaybackGraphFactory : public UFactory
{
	GENERATED_BODY()

public:
	UYSPlaybackGraphFactory();

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};
