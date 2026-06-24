// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "YSPlayerController.generated.h"

class UYSCameraLockOnComponent;
class UInputMappingContext;
/**
 * 
 */
UCLASS()
class MNYS_API AYSPlayerController : public APlayerController
{
	GENERATED_BODY()

public :
	AYSPlayerController(const FObjectInitializer& ObjectInitializer);
	virtual void SetupInputComponent() override;
	
protected : 
	virtual void AddYawInput(float Val) override;
	
public :
	UPROPERTY(EditDefaultsOnly, Category="Input|Input Mappings")
	TObjectPtr<UInputMappingContext> UIInputMappingContext;
	
	UPROPERTY(EditDefaultsOnly, Category="YS | Lock On")
	TObjectPtr<UYSCameraLockOnComponent> LockOnComponent;
	
protected : 
	UPROPERTY(EditDefaultsOnly)
	float LockOnBreakThreshold = 3.0f; 
};
