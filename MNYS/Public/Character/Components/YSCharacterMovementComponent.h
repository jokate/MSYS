// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <General/YSMacros.h>

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
	static UYSCharacterMovementComponent* Get(AActor* InActor);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	void SetMovementBlocked(bool bBlocked) { bIsMovementBlocked = bBlocked; }

	bool IsMovementBlocked() const { return bIsMovementBlocked; }
	
	YS_ACCESSOR(FVector, LocalSpaceVelocity);
	
protected :
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector LocalSpaceVelocity = FVector::ZeroVector;
	
private :
	bool bIsMovementBlocked = false;
};
