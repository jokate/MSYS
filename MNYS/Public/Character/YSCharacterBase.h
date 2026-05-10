// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "YSCharacterBase.generated.h"

class UYSCharacterMovementComponent;
class UYSMotionWarpingComponent;
class UYSAbilitySystemComponent;
class UCameraComponent;
class USpringArmComponent;

UCLASS()
class MNYS_API AYSCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AYSCharacterBase(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual UYSCharacterMovementComponent* GetYSCharacterMovement();

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;


public :
	UPROPERTY(BlueprintReadWrite, Category = "MotionWarping", EditDefaultsOnly )
	TObjectPtr<UYSMotionWarpingComponent> MotionWarpingComponent;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "AbilitySystemComponent")
	TObjectPtr<UYSAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon")
	TObjectPtr<USkeletalMeshComponent> WeaponMeshComponent;
};
