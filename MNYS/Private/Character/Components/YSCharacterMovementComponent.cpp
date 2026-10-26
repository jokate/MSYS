// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Components/YSCharacterMovementComponent.h"

#include "YSAbilitySystemComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Character/YSCharacterBase.h"
#include "Character/YSPlayerController.h"
#include "Character/Components/YSCombatTargetComponent.h"
#include "Curves/CurveFloat.h"
#include "Data/YSCombatProfile.h"
#include "General/YSGameplayTag.h"
#include "Input/StateMachine/YSInputStateMachineComponent.h"
#include "MotionWarp/UYSMotionWarpingComponent.h"


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

	// 애님 루트 모션 중엔 엔진이 PhysicsRotation 을 부르지 않는다. 어빌리티 중에도 보간이 돌도록 Tick 에서 민다.
	TickTurn(DeltaTime);

	if ( PendingFaceYaw.IsSet() && IsTurning() == false )
	{
		const UYSCombatProfile* Profile = ResolveTurnProfile();

		if ( Profile == nullptr || IsTurnBlocked(*Profile) == false )
		{
			const FPendingFaceYaw Pending = PendingFaceYaw.GetValue();
			PendingFaceYaw.Reset();
			RequestFaceYaw(Pending.Yaw, Pending.Source);
		}
	}
}

void UYSCharacterMovementComponent::PhysicsRotation(float DeltaTime)
{
	// 보간 턴이 몸 yaw 를 쥐고 있다. orient-to-movement 가 끼면 매 틱 되돌린다.
	if ( bInterpTurning )
	{
		return;
	}

	Super::PhysicsRotation(DeltaTime);
}

void UYSCharacterMovementComponent::RequestFaceYaw(float TargetYaw, EYSFaceYawSource Source)
{
	if ( HasValidData() == false )
	{
		return;
	}

	const UYSCombatProfile* Profile = ResolveTurnProfile();
	const FYSTurnSourceRule* Rule = Profile ? Profile->TurnSourceRules.Find(Source) : nullptr;

	if ( Rule == nullptr )
	{
		CancelTurn();
		PendingFaceYaw.Reset();
		SetFaceYaw(TargetYaw);
		return;
	}

	if ( Rule->bRespectBlockedTags && IsTurnBlocked(*Profile) )
	{
		PendingFaceYaw = FPendingFaceYaw{ TargetYaw, Source };
		return;
	}

	PendingFaceYaw.Reset();
	StartTurn(*Profile, *Rule, TargetYaw, Source);
}

const UYSCombatProfile* UYSCharacterMovementComponent::ResolveTurnProfile() const
{
	// 플레이어만 프로필을 갖는다. 몬스터는 nullptr 로 즉시 회전(이전 동작)을 유지한다.
	const AYSPlayerController* PC = CharacterOwner ? Cast<AYSPlayerController>(CharacterOwner->GetController()) : nullptr;

	if ( PC == nullptr || IsValid(PC->CombatTargetComponent) == false )
	{
		return nullptr;
	}

	return PC->CombatTargetComponent->GetActiveProfile();
}

bool UYSCharacterMovementComponent::IsTurnBlocked(const UYSCombatProfile& Profile) const
{
	if ( Profile.TurnBlockedTags.IsEmpty() == false )
	{
		const UYSAbilitySystemComponent* ASC = UYSAbilitySystemComponent::Get(GetOwner());

		if ( IsValid(ASC) && ASC->HasAnyMatchingGameplayTags(Profile.TurnBlockedTags) )
		{
			return true;
		}
	}

	// 입력 상태 머신이 없는 액터는 막지 않는다.
	const UYSInputStateMachineComponent* InputStateMachine = UYSInputStateMachineComponent::Get(GetOwner());

	if ( IsValid(InputStateMachine) == false )
	{
		return false;
	}

	return Profile.TurnBlockedInputStates.ContainsByPredicate([InputStateMachine](EYSInputStatesType State)
	{
		return InputStateMachine->IsStateRequested(State);
	});
}

bool UYSCharacterMovementComponent::IsMovingForTurn() const
{
	return GetCurrentAcceleration().IsNearlyZero() == false || Velocity.SizeSquared2D() > UE_KINDA_SMALL_NUMBER;
}

void UYSCharacterMovementComponent::StartTurn(const UYSCombatProfile& Profile, const FYSTurnSourceRule& Rule, float TargetYaw, EYSFaceYawSource Source)
{
	CancelTurn();

	const float CurrentYaw = UpdatedComponent->GetComponentRotation().Yaw;
	const float DeltaYaw = FRotator::NormalizeAxis(TargetYaw - CurrentYaw);
	const FYSTurnBand* Band = Profile.FindTurnBand(FMath::Abs(DeltaYaw));

	if ( Band == nullptr )
	{
		SetFaceYaw(TargetYaw);
		return;
	}

	TurnTargetYaw = TargetYaw;
	TurnSource = Source;
	bTurnRespectsBlock = Rule.bRespectBlockedTags;

	const bool bMontageAllowed = Rule.bAllowTurnMontage && ( Profile.bTurnMontageWhileMoving || IsMovingForTurn() == false );

	if ( bMontageAllowed && TryPlayTurnMontage(Profile, *Band, DeltaYaw, TargetYaw) )
	{
		return;
	}

	float Duration = Band->Duration / FMath::Max(Band->PlayRate, UE_KINDA_SMALL_NUMBER);

	if ( Rule.MaxDuration > 0.f )
	{
		Duration = FMath::Min(Duration, Rule.MaxDuration);
	}

	if ( Duration <= UE_KINDA_SMALL_NUMBER )
	{
		SetFaceYaw(TargetYaw);
		return;
	}

	bInterpTurning = true;
	TurnStartYaw = CurrentYaw;
	TurnDeltaYaw = DeltaYaw;
	TurnElapsed = 0.f;
	TurnDuration = Duration;
	TurnCurve = Band->Curve;
}

bool UYSCharacterMovementComponent::TryPlayTurnMontage(const UYSCombatProfile& Profile, const FYSTurnBand& Band, float DeltaYaw, float TargetYaw)
{
	// 양의 yaw 가 오른쪽 회전이다.
	UAnimMontage* Montage = ( DeltaYaw < 0.f ) ? Band.MontageLeft.Get() : Band.MontageRight.Get();
	USkeletalMeshComponent* Mesh = CharacterOwner ? CharacterOwner->GetMesh() : nullptr;
	UAnimInstance* AnimInstance = Mesh ? Mesh->GetAnimInstance() : nullptr;

	if ( IsValid(Montage) == false || IsValid(AnimInstance) == false )
	{
		return false;
	}

	// 몽타주 루트 yaw 가 구간 대표각(90 · 180)이라, 중간 각은 워핑이 목표 yaw 까지 늘린다.
	UYSMotionWarpingComponent* MotionWarping = UYSMotionWarpingComponent::GetMotionWarpingComponent(GetOwner());

	if ( IsValid(MotionWarping) && Profile.TurnWarpTargetName.IsNone() == false )
	{
		MotionWarping->AddOrUpdateWarpTargetFromLocationAndRotation(Profile.TurnWarpTargetName,
			UpdatedComponent->GetComponentLocation(), FRotator(0.f, TargetYaw, 0.f));
		ActiveTurnWarpTargetName = Profile.TurnWarpTargetName;
	}

	if ( AnimInstance->Montage_Play(Montage, Band.PlayRate) <= 0.f )
	{
		if ( IsValid(MotionWarping) && ActiveTurnWarpTargetName.IsNone() == false )
		{
			MotionWarping->RemoveWarpTarget(ActiveTurnWarpTargetName);
		}
		ActiveTurnWarpTargetName = NAME_None;
		return false;
	}

	ActiveTurnMontage = Montage;

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &UYSCharacterMovementComponent::HandleTurnMontageEnded);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, Montage);
	return true;
}

void UYSCharacterMovementComponent::TickTurn(float DeltaTime)
{
	if ( bInterpTurning == false )
	{
		return;
	}

	// 보간 중에 공격 · 회피가 시작되면 멈추고, 끝난 뒤 남은 회전을 다시 한다.
	if ( bTurnRespectsBlock )
	{
		const UYSCombatProfile* Profile = ResolveTurnProfile();

		if ( Profile && IsTurnBlocked(*Profile) )
		{
			bInterpTurning = false;
			DeferTurn(TurnTargetYaw, TurnSource);
			return;
		}
	}

	TurnElapsed += DeltaTime;

	const float Alpha = FMath::Clamp(TurnElapsed / TurnDuration, 0.f, 1.f);
	const float Eased = TurnCurve ? TurnCurve->GetFloatValue(Alpha) : FMath::SmoothStep(0.f, 1.f, Alpha);

	SetFaceYaw(TurnStartYaw + TurnDeltaYaw * Eased);

	if ( Alpha >= 1.f )
	{
		bInterpTurning = false;
		TurnCurve = nullptr;
	}
}

void UYSCharacterMovementComponent::CancelTurn()
{
	bInterpTurning = false;
	TurnCurve = nullptr;

	UAnimMontage* Montage = ActiveTurnMontage.Get();

	if ( Montage == nullptr )
	{
		return;
	}

	// 먼저 비워야 종료 콜백이 "끊겼다" 로 다시 대기시키지 않는다.
	ActiveTurnMontage.Reset();

	if ( UYSMotionWarpingComponent* MotionWarping = UYSMotionWarpingComponent::GetMotionWarpingComponent(GetOwner()) )
	{
		MotionWarping->RemoveWarpTarget(ActiveTurnWarpTargetName);
	}
	ActiveTurnWarpTargetName = NAME_None;

	USkeletalMeshComponent* Mesh = CharacterOwner ? CharacterOwner->GetMesh() : nullptr;

	if ( UAnimInstance* AnimInstance = Mesh ? Mesh->GetAnimInstance() : nullptr )
	{
		AnimInstance->Montage_Stop(Montage->GetDefaultBlendOutTime(), Montage);
	}
}

void UYSCharacterMovementComponent::SetFaceYaw(float Yaw)
{
	// yaw 만 돌린다. 락온 정책의 높이 차 pitch 는 몸에 싣지 않는다.
	FRotator Rotation = UpdatedComponent->GetComponentRotation();
	Rotation.Yaw = Yaw;
	MoveUpdatedComponent(FVector::ZeroVector, Rotation.Quaternion(), false);
}

void UYSCharacterMovementComponent::DeferTurn(float TargetYaw, EYSFaceYawSource Source)
{
	// 대기 중에 들어온 더 새 요청이 있으면 그쪽이 이긴다.
	if ( PendingFaceYaw.IsSet() == false )
	{
		PendingFaceYaw = FPendingFaceYaw{ TargetYaw, Source };
	}
}

void UYSCharacterMovementComponent::HandleTurnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if ( Montage == nullptr || Montage != ActiveTurnMontage.Get() )
	{
		return;
	}

	ActiveTurnMontage.Reset();

	if ( UYSMotionWarpingComponent* MotionWarping = UYSMotionWarpingComponent::GetMotionWarpingComponent(GetOwner()) )
	{
		MotionWarping->RemoveWarpTarget(ActiveTurnWarpTargetName);
	}
	ActiveTurnWarpTargetName = NAME_None;

	// 막는 상태(공격 · 회피)가 끊었으면 남은 회전을 그 뒤로 미룬다. 그 밖의 몽타주가 끊었으면 그쪽이 이긴다.
	if ( bInterrupted && bTurnRespectsBlock )
	{
		const UYSCombatProfile* Profile = ResolveTurnProfile();

		if ( Profile && IsTurnBlocked(*Profile) )
		{
			DeferTurn(TurnTargetYaw, TurnSource);
		}
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

void UYSCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
	
	if ( MovementMode == MOVE_Falling )
	{
		UYSInputStateMachineComponent* InputStateMachine = UYSInputStateMachineComponent::Get(GetOwner());
		if ( IsValid(InputStateMachine) )
		{
			InputStateMachine->AddStateStack(EYSInputStatesType::Falling);
		}
	}
	
	if ( PreviousMovementMode == MOVE_Falling )
	{
		UYSInputStateMachineComponent* InputStateMachine = UYSInputStateMachineComponent::Get(GetOwner());
		if ( IsValid(InputStateMachine) )
		{
			InputStateMachine->RemoveStateStack(EYSInputStatesType::Falling);
		}
		
		UYSAbilitySystemComponent* ASC = UYSAbilitySystemComponent::Get(GetOwner());
		
		if ( IsValid(ASC) )
		{
			ASC->ClearAirUsage();
		}
		
		FGameplayEventData EventData;
		// 이벤트 발생.
		ASC->HandleGameplayEvent(YSTags::Event_OnLand, &EventData);
	}
}

