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

	virtual bool HandlePendingLaunch() override;
	
	void SetMovementBlocked(bool bBlocked) 
	{ 
		if (bBlocked)
		{
			++MovementBlockSemaphore;
		}
		else
		{
			--MovementBlockSemaphore;
			if (MovementBlockSemaphore < 0)
			{
				UE_LOG(LogTemp, Warning, TEXT("MovementBlockSemaphore is negative! Check SetMovementBlocked calls."));
				MovementBlockSemaphore = 0; // 안전장치
			}
		}
	}

	bool IsMovementBlocked() const { return MovementBlockSemaphore > 0; }
	
	YS_ACCESSOR(FVector, LocalSpaceVelocity);
	YS_GETTER(FVector, LocalSpaceLastInputDirection);

protected :
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector LocalSpaceVelocity = FVector::ZeroVector;

	// 마지막 비-Zero 입력 방향 (로컬 스페이스, 정규화). 입력이 없어도 직전 값 유지.
	FVector LocalSpaceLastInputDirection = FVector::ForwardVector;
	
private :
	int32 MovementBlockSemaphore = 0;
};
