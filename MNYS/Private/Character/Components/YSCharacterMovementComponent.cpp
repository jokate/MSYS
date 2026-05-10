// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Components/YSCharacterMovementComponent.h"

#include "Character/YSCharacterBase.h"


// Sets default values for this component's properties
UYSCharacterMovementComponent::UYSCharacterMovementComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

UYSCharacterMovementComponent* UYSCharacterMovementComponent::Get(AActor* InActor)
{
	AYSCharacterBase* CharBase = Cast<AYSCharacterBase>(InActor);

	if ( IsValid(CharBase) == false )
		return nullptr;

	return CharBase->GetYSCharacterMovement();
}


// Called when the game starts
void UYSCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UYSCharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                  FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
	LocalSpaceVelocity = FVector(0, 0, 0);
	if ( Velocity.IsNearlyZero() == false )
	{
		FTransform OriginalTransform = UpdatedComponent->GetComponentTransform();
		LocalSpaceVelocity = OriginalTransform.InverseTransformVectorNoScale(Velocity);	
	}
}

