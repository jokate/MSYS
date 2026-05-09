// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "YSCharacterMovementComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MNYS_API UYSCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UYSCharacterMovementComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

public :
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector LocalSpaceVelocity = FVector::ZeroVector;

	void SetMovementBlocked(bool bBlocked) { bIsMovementBlocked = bBlocked; }
	virtual float GetMaxSpeed() const override;

private :
	bool bIsMovementBlocked = false;
};
