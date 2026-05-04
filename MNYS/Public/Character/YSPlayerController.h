// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "YSPlayerController.generated.h"

class UInputMappingContext;
/**
 * 
 */
UCLASS()
class MNYS_API AYSPlayerController : public APlayerController
{
	GENERATED_BODY()

public :
	virtual void SetupInputComponent() override;
	
public :
	UPROPERTY(EditDefaultsOnly, Category="Input|Input Mappings")
	TObjectPtr<UInputMappingContext> UIInputMappingContext;
};
