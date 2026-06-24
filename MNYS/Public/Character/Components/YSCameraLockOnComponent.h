// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "General/YSStruct.h"
#include "YSCameraLockOnComponent.generated.h"


class AYSCharacterPlayer;
class AYSPlayerController;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MNYS_API UYSCameraLockOnComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UYSCameraLockOnComponent();
	static UYSCameraLockOnComponent* Get(const AActor* Character);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	virtual void ForceSetLockOn(AActor* TargetToLockOn);
	virtual void ReleaseLockOn()
	{
		CurrentLockedTarget = nullptr;
	}
	
	virtual AActor* GetCurrentTarget() { return CurrentLockedTarget.Get(); }
protected :
	void ProcessLockOnFunction(float DeltaTime);
	void ChaseCamera(float DeltaTime);

public : 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (DisplayName = "현재 잠금된 대상"))
	TWeakObjectPtr<AActor> CurrentLockedTarget;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (DisplayName = "잠금 대상이 될 수 있는 최대 각도"))
	float CameraInterpSpeed = 0.3f;
	
	UPROPERTY()
	TWeakObjectPtr<AYSPlayerController> OwnerPlayerController;

	UPROPERTY()
	TWeakObjectPtr<AYSCharacterPlayer> OwnerPlayer;

public:
	void StartCameraEffect(const FYSCameraEffectParams& Params);
	void StopCameraEffect();

private:
	void TickCameraEffect(float DeltaTime);

	FYSCameraEffectParams ActiveCameraParams;

	bool bCameraEffectActive = false;
	bool bCameraRestoring    = false;

	float DefaultArmLength    = 600.f;
	FVector DefaultSocketOffset = FVector::ZeroVector;
	float DefaultFOV          = 90.f;
	FRotator DefaultRotation = FRotator::ZeroRotator;
};
