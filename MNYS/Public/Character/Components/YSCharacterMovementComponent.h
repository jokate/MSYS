// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <General/YSMacros.h>

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
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
	UYSCharacterMovementComponent();
	static UYSCharacterMovementComponent* Get(AActor* InActor);

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	virtual bool HandlePendingLaunch() override;
	
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	/** 요청자가 하나라도 남아 있으면 중력 0. 마지막 요청자가 풀면 끄기 전 값으로 돌린다. */
	void SuspendGravity(const UObject* Requester);
	void ResumeGravity(const UObject* Requester);

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
				MovementBlockSemaphore = 0;
			}
		}
	}

	bool IsMovementBlocked() const { return MovementBlockSemaphore > 0; }
	
	YS_ACCESSOR(FVector, LocalSpaceVelocity);
	YS_GETTER(FVector, LocalSpaceLastInputDirection);

	/** 몸 yaw 를 목표로 돌린다 (모든 회전이 여기로 온다). 구간 · 출처 규칙은 플레이어 전투 프로필에서 고르고, 프로필이나 출처 행이 없으면 즉시 돈다. */
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
	
	// 낙하하는 동안 거는 입력 상태. 이동이 만드는 상태라 이 컴포넌트가 걸고 뗀다.
	UPROPERTY(EditDefaultsOnly, Category = "YS | Input", meta = (DisplayName = "낙하 입력 상태", GetOptions = "MNYS.YSDeveloperSettings.GetInputStateOptions"))
	FName FallingInputState = TEXT("Falling");

private :
	int32 MovementBlockSemaphore = 0;

	TArray<TWeakObjectPtr<const UObject>> GravitySuspenders;
	float SuspendedGravityScale = 1.f;

	FActiveGameplayEffectHandle FallingStateHandle;
};
