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
	const FTransform& OwnerTransform = UpdatedComponent->GetComponentTransform();

	LocalSpaceVelocity = Velocity.IsNearlyZero()
		? FVector::ZeroVector
		: OwnerTransform.InverseTransformVectorNoScale(Velocity);

	// Super 이후 LastInputVector에 이번 프레임 소비된 입력이 보존됨
	const FVector WorldInput = GetLastInputVector();
	if (!WorldInput.IsNearlyZero())
	{
		LocalSpaceLastInputDirection = OwnerTransform.InverseTransformVectorNoScale(WorldInput).GetSafeNormal();
	}
}

bool UYSCharacterMovementComponent::HandlePendingLaunch()
{
	if (!PendingLaunchVelocity.IsZero() && HasValidData())
	{
		Velocity = PendingLaunchVelocity;
		
		// 런칭 자체는 사실 Falling이 맞긴한데, Z축으로 힘준 경우에만 Falling 처리.
		if ( PendingLaunchVelocity.Z > 0.f )
		{
			SetMovementMode(MOVE_Falling);	
		}
		
		PendingLaunchVelocity = FVector::ZeroVector;
		bForceNextFloorCheck = true;
		return true;
	}

	return false;
}

