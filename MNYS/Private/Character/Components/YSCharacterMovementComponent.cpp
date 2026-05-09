// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Components/YSCharacterMovementComponent.h"


// Sets default values for this component's properties
UYSCharacterMovementComponent::UYSCharacterMovementComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UYSCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
float UYSCharacterMovementComponent::GetMaxSpeed() const
{
	if (bIsMovementBlocked)
		return 0.f;

	return Super::GetMaxSpeed();
}

void UYSCharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                  FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
	FTransform OriginalTransform = UpdatedComponent->GetComponentTransform();
	LocalSpaceVelocity = OriginalTransform.InverseTransformPosition(Velocity);
}

