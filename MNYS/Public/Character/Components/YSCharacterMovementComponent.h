// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <General/YSMacros.h>

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "General/YSEnum.h"
#include "YSCharacterMovementComponent.generated.h"

class UAnimMontage;
class UCurveFloat;
class UYSCombatProfile;
struct FYSTurnBand;
struct FYSTurnSourceRule;


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
	
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	void SetGravityScale(float InGravityScale)
	{
		GravityScale = InGravityScale;
	}
	
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

	/**
	 * 몸 yaw 를 목표로 돌린다. 몸을 돌리는 곳은 모두 여기로 온다.
	 * 각도 구간 · 출처 규칙은 플레이어 전투 프로필에서 고른다. 프로필이나 출처 행이 없으면 즉시 돈다.
	 */
	void RequestFaceYaw(float TargetYaw, EYSFaceYawSource Source);

	bool IsTurning() const { return bInterpTurning || ActiveTurnMontage.IsValid(); }

protected :
	virtual void PhysicsRotation(float DeltaTime) override;

private :
	const UYSCombatProfile* ResolveTurnProfile() const;
	bool IsTurnBlocked(const UYSCombatProfile& Profile) const;
	bool IsMovingForTurn() const;

	void StartTurn(const UYSCombatProfile& Profile, const FYSTurnSourceRule& Rule, float TargetYaw, EYSFaceYawSource Source);
	bool TryPlayTurnMontage(const UYSCombatProfile& Profile, const FYSTurnBand& Band, float DeltaYaw, float TargetYaw);
	void TickTurn(float DeltaTime);
	void CancelTurn();
	void SetFaceYaw(float Yaw);
	void DeferTurn(float TargetYaw, EYSFaceYawSource Source);
	void HandleTurnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	struct FPendingFaceYaw
	{
		float Yaw = 0.f;
		EYSFaceYawSource Source = EYSFaceYawSource::TargetSwitch;
	};

	// 막혀 있는 동안 들어온 마지막 요청 1칸.
	TOptional<FPendingFaceYaw> PendingFaceYaw;

	bool bInterpTurning = false;
	float TurnStartYaw = 0.f;
	float TurnDeltaYaw = 0.f;
	float TurnElapsed = 0.f;
	float TurnDuration = 0.f;

	// 진행 중인 턴의 목표 · 출처 · 규칙. 막혀서 끊기면 이 값으로 다시 대기한다.
	float TurnTargetYaw = 0.f;
	EYSFaceYawSource TurnSource = EYSFaceYawSource::TargetSwitch;
	bool bTurnRespectsBlock = false;

	UPROPERTY()
	TObjectPtr<UCurveFloat> TurnCurve;

	TWeakObjectPtr<UAnimMontage> ActiveTurnMontage;
	FName ActiveTurnWarpTargetName;

protected :
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector LocalSpaceVelocity = FVector::ZeroVector;

	// 마지막 비-Zero 입력 방향 (로컬 스페이스, 정규화). 입력이 없어도 직전 값 유지.
	FVector LocalSpaceLastInputDirection = FVector::ForwardVector;
	
private :
	int32 MovementBlockSemaphore = 0;
};
