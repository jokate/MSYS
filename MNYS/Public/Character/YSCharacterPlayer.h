// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InputActionValue.h"
#include "YSCharacterBase.h"
#include "General/YSEnum.h"
#include "YSCharacterPlayer.generated.h"

class UInputMappingContext;
class UYSInputConfig;
class UYSInputStateMachineComponent;

UCLASS()
class MNYS_API AYSCharacterPlayer : public AYSCharacterBase
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AYSCharacterPlayer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	void ProcessInput(FGameplayTag InputTag);

	void TransitionStateMachine(EYSInputStatesType InputStateType) const;
	
	void Look(const FInputActionValue& Value);
	void Move(const FInputActionValue& Value);
	virtual void PossessedBy(AController* NewController) override;

protected : 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = InputControl)
	TObjectPtr<UYSInputStateMachineComponent> InputStateMachineComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = InputControl)
	TObjectPtr<UYSInputConfig> InputConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = InputControl)
	TObjectPtr<UInputMappingContext> PlayerInputMappingContext;
};

