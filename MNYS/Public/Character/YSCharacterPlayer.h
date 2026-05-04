// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "YSCharacterBase.h"
#include "YSCharacterPlayer.generated.h"

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
	void ProcessInput(const FGameplayTag& InputTag) const;

protected : 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = InputControl)
	TObjectPtr<UYSInputStateMachineComponent> InputStateMachineComponent;
};
