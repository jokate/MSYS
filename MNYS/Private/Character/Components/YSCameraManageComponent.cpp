// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Components/YSCameraManageComponent.h"

#include "YSAbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Character/YSCharacterPlayer.h"
#include "Character/YSPlayerController.h"
#include "Character/Components/YSCharacterMovementComponent.h"
#include "Character/Components/YSCombatTargetComponent.h"
#include "Data/YSTransformPolicy.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "General/YSGameplayTag.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Library/YSBlueprintFunctionLibrary.h"
#include "General/YSDefine.h"
#include "Subsystem/YSSquadSubsystem.h"
#include "YSDeveloperSettings.h"


UYSCameraManageComponent::UYSCameraManageComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

UYSCameraManageComponent* UYSCameraManageComponent::Get(const AActor* Character)
{
	if ( IsValid(Character) == false )
		return nullptr;

	AYSPlayerController* PC = Character->GetInstigatorController<AYSPlayerController>();

	if ( IsValid(PC) == false )
		return nullptr;

	return PC->LockOnComponent;
}


void UYSCameraManageComponent::BeginPlay()
{
	Super::BeginPlay();

	AYSPlayerController* PlayerController = Cast<AYSPlayerController>(GetOwner());

	if ( IsValid(PlayerController) )
	{
		OwnerPlayer = Cast<AYSCharacterPlayer>(PlayerController->GetPawn());
		OwnerPlayerController = PlayerController;

		// 스쿼드 태그 교대로 폰이 바뀐다. 여기서 다시 잡지 않으면 벤치로 내려간 이전 폰의 카메라를 만지게 된다.
		PlayerController->OnPossessedPawnChanged.AddDynamic(this, &UYSCameraManageComponent::HandlePossessedPawnChanged);

		if ( UYSCombatTargetComponent* CombatTarget = PlayerController->CombatTargetComponent )
		{
			CombatTarget->OnLockOnChanged.AddDynamic(this, &UYSCameraManageComponent::HandleLockOnChanged);
		}
	}

	if ( UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr )
	{
		if ( UYSSquadSubsystem* Squad = GameInstance->GetSubsystem<UYSSquadSubsystem>() )
		{
			Squad->OnSwitchWindowOpened.AddDynamic(this, &UYSCameraManageComponent::HandleSwitchWindowOpened);
		}
	}

	CaptureCameraDefaults();
}

void UYSCameraManageComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 스쿼드는 게임 인스턴스 수명이라 컨트롤러보다 오래 산다. 레벨이 바뀌어도 죽은 구독이 남지 않게 끊는다.
	if ( UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr )
	{
		if ( UYSSquadSubsystem* Squad = GameInstance->GetSubsystem<UYSSquadSubsystem>() )
		{
			Squad->OnSwitchWindowOpened.RemoveDynamic(this, &UYSCameraManageComponent::HandleSwitchWindowOpened);
		}
	}

	if ( UWorld* World = GetWorld() )
	{
		World->GetTimerManager().ClearTimer(ImpactZoomTimer);
	}

	Super::EndPlay(EndPlayReason);
}

void UYSCameraManageComponent::HandleSwitchWindowOpened(EYSSwitchTrigger Trigger, AActor* Target, float AbuseCooldown)
{
	const EYSImpactGrade Grade = GetDefault<UYSDeveloperSettings>()->GetSwitchWindowImpact(Trigger);

	if ( Grade == EYSImpactGrade::None || OwnerPlayerController.IsValid() == false )
	{
		return;
	}

	AActor* Instigator = OwnerPlayerController->GetPawn();
	TArray<AActor*> Targets;

	if ( IsValid(Target) )
	{
		Targets.Add(Target);
	}

	UYSBlueprintFunctionLibrary::PlayImpact(Instigator, Targets, Grade);
}

void UYSCameraManageComponent::HandleLockOnChanged(AActor* LockedTarget)
{
	UE_LOG(LogYSMeasure, Verbose, TEXT("[LockOn] t=%.3f Target=%s"), GetWorld()->GetTimeSeconds(), *GetNameSafe(LockedTarget));

	if ( IsValid(LockedTarget) == false )
	{
		PopCameraMode(this);
		return;
	}

	if ( OwnerPlayer.IsValid() == false )
	{
		return;
	}

	const FRotator CharacterRotation = FYSRotationPolicy_LockOnTarget().GetRotation(FYSTransformPolicyContext(OwnerPlayer.Get()));

	if ( UYSCharacterMovementComponent* Movement = UYSCharacterMovementComponent::Get(OwnerPlayer.Get()) )
	{
		Movement->RequestFaceYaw(CharacterRotation.Yaw, EYSFaceYawSource::TargetSwitch);
	}
	else
	{
		OwnerPlayer->SetActorRotation(CharacterRotation);
	}

	// 락온도 카메라 요청자 중 하나다. 조준·연계기가 위에 얹히면 그쪽이 이기고, 빠지면 락온 프로파일로 돌아온다.
	if ( bUseLockOnCameraMode )
	{
		PushCameraMode(this, LockOnCameraParams, EYSCameraModePriority::LockOn);
	}
}

AActor* UYSCameraManageComponent::GetLockedTarget() const
{
	const UYSCombatTargetComponent* CombatTarget = OwnerPlayerController.IsValid() ? OwnerPlayerController->CombatTargetComponent.Get() : nullptr;
	return IsValid(CombatTarget) ? CombatTarget->GetLockedTarget() : nullptr;
}

void UYSCameraManageComponent::HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	OwnerPlayer = Cast<AYSCharacterPlayer>(NewPawn);

	// 새 캐릭터의 기본값을 복귀 지점으로 다시 잡는다. 암 길이·FOV가 캐릭터마다 달라 이전 값으로 복귀하면 엉뚱한 화각이 된다.
	CaptureCameraDefaults();

	// 교대 시점에 요청이 남아 있으면 새 폰이 그 프로파일로 다시 블렌드해 들어가야 한다.
	bCameraSettled = false;
}

void UYSCameraManageComponent::CaptureCameraDefaults()
{
	if ( OwnerPlayer.IsValid() == false )
	{
		return;
	}

	if ( USpringArmComponent* Boom = OwnerPlayer->GetCameraBoom() )
	{
		DefaultArmLength = Boom->TargetArmLength;
		DefaultSocketOffset = Boom->SocketOffset;
	}

	if ( UCameraComponent* Cam = OwnerPlayer->GetFollowCamera() )
	{
		DefaultFOV      = Cam->FieldOfView;
		DefaultRotation = Cam->GetRelativeRotation();
	}
	
	if ( OwnerPlayer.IsValid() )
	{
		bOriginControl = OwnerPlayer->GetCharacterMovement()->bOrientRotationToMovement;
		bControlYaw = OwnerPlayer->bUseControllerRotationYaw;
	}
}


void UYSCameraManageComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ChaseCamera(GetLockedTarget(), DeltaTime);
	TickCameraEffect(DeltaTime);
}

void UYSCameraManageComponent::ChaseCamera(AActor* LockedTarget, float DeltaTime)
{
	if ( OwnerPlayerController.IsValid() == false || OwnerPlayer.IsValid() == false || IsValid(LockedTarget) == false )
		return;

	FVector ToTarget = (LockedTarget->GetActorLocation() - OwnerPlayer->GetActorLocation()).GetSafeNormal();
	FRotator TargetRot  = ToTarget.Rotation();
	FRotator CurrentRot = OwnerPlayerController->GetControlRotation();

	TargetRot.Pitch = CurrentRot.Pitch;
	OwnerPlayerController->SetControlRotation(
		FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, CameraInterpSpeed));
}

void UYSCameraManageComponent::PushCameraMode(UObject* Requester, const FYSCameraEffectParams& Params, EYSCameraModePriority Priority)
{
	if ( IsValid(Requester) == false )
	{
		return;
	}

	// 같은 요청자가 다시 부르면 갱신이다. 중복 엔트리를 만들면 Pop 한 번으로 정리되지 않는다.
	for ( FYSCameraModeRequest& Request : CameraModeStack )
	{
		if ( Request.Requester == Requester )
		{
			Request.Params = Params;
			Request.Priority = Priority;
			bCameraSettled = false;
			return;
		}
	}

	FYSCameraModeRequest NewRequest;
	NewRequest.Requester = Requester;
	NewRequest.Params = Params;
	NewRequest.Priority = Priority;

	CameraModeStack.Add(MoveTemp(NewRequest));
	bCameraSettled = false;
}

void UYSCameraManageComponent::PopCameraMode(UObject* Requester)
{
	const int32 Index = CameraModeStack.IndexOfByPredicate(
		[Requester](const FYSCameraModeRequest& Request)
		{
			return Request.Requester == Requester;
		});

	if ( Index == INDEX_NONE )
	{
		return;
	}

	// 복귀 속도는 빠지는 요청의 값을 쓴다. 남은 요청이 있으면 아래에서 그쪽으로 다시 덮인다.
	RestoreInterpSpeed = CameraModeStack[Index].Params.InterpOutSpeed;

	CameraModeStack.RemoveAt(Index);
	bCameraSettled = false;
}

bool UYSCameraManageComponent::HasCameraMode(const UObject* Requester) const
{
	return CameraModeStack.ContainsByPredicate(
		[Requester](const FYSCameraModeRequest& Request)
		{
			return Request.Requester == Requester;
		});
}

void UYSCameraManageComponent::StartCameraEffect(UObject* Requester, const FYSCameraEffectParams& Params)
{
	PushCameraMode(Requester, Params, EYSCameraModePriority::Default);
}

void UYSCameraManageComponent::StopCameraEffect(UObject* Requester)
{
	PopCameraMode(Requester);
}

void UYSCameraManageComponent::PlayImpactZoom(const FYSImpactProfile& Profile)
{
	UWorld* World = GetWorld();

	if ( Profile.ZoomDuration <= 0.f || IsValid(World) == false )
	{
		return;
	}

	if ( IsValid(ImpactZoomRequester) == false )
	{
		ImpactZoomRequester = NewObject<UObject>(this);
	}

	// 죽은 요청을 먼저 걷고, 임팩트 줌 자신을 뺀 최상위 모드를 기준으로 삼는다. 자신을 넣으면 다시 불릴 때 증감이 쌓인다.
	ResolveActiveMode();

	const FYSCameraModeRequest* Base = nullptr;

	for ( int32 Index = CameraModeStack.Num() - 1; Index >= 0; --Index )
	{
		if ( CameraModeStack[Index].Requester == ImpactZoomRequester )
		{
			continue;
		}

		if ( Base == nullptr || CameraModeStack[Index].Priority > Base->Priority )
		{
			Base = &CameraModeStack[Index];
		}
	}

	FYSCameraEffectParams Params;

	if ( Base != nullptr )
	{
		Params = Base->Params;
	}
	else
	{
		Params.TargetArmLength = DefaultArmLength;
		Params.SocketOffset = DefaultSocketOffset;
		Params.FieldOfView = DefaultFOV;
		Params.RelativeRotator = DefaultRotation;
		Params.bOrientToControlRotation = bOriginControl;
		Params.bUseControllerRotationYaw = bControlYaw;
	}

	Params.TargetArmLength += Profile.ZoomArmLengthDelta;
	Params.SocketOffset.Z += Profile.ZoomHeightDelta;
	Params.FieldOfView += Profile.ZoomFOVDelta;
	Params.InterpInSpeed = Profile.ZoomInterpInSpeed;
	Params.InterpOutSpeed = Profile.ZoomInterpOutSpeed;

	PushCameraMode(ImpactZoomRequester, Params, EYSCameraModePriority::Impact);

	World->GetTimerManager().SetTimer(ImpactZoomTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		PopCameraMode(ImpactZoomRequester);
	}), Profile.ZoomDuration, false);
}

const FYSCameraModeRequest* UYSCameraManageComponent::ResolveActiveMode()
{
	// 요청자가 사라졌으면 요청도 무효다 (EndAbility 없이 파괴된 어빌리티가 카메라를 물고 늘어지는 것을 막는다). 고르기 전에 걷어야 Best 포인터가 RemoveAt 에 밀리지 않는다.
	const int32 NumRemoved = CameraModeStack.RemoveAll(
		[](const FYSCameraModeRequest& Request)
		{
			return Request.Requester.IsValid() == false;
		});

	if ( NumRemoved > 0 )
	{
		bCameraSettled = false;
	}

	const FYSCameraModeRequest* Best = nullptr;

	for ( int32 Index = CameraModeStack.Num() - 1; Index >= 0; --Index )
	{
		// 뒤에서부터 순회하므로, 우선순위가 같으면 나중에 들어온 요청이 남는다.
		if ( Best == nullptr || CameraModeStack[Index].Priority > Best->Priority )
		{
			Best = &CameraModeStack[Index];
		}
	}

	return Best;
}

void UYSCameraManageComponent::TickControlPitch(const FYSCameraEffectParams* Params, float DeltaTime, float Speed)
{
	if ( OwnerPlayerController.IsValid() == false )
	{
		return;
	}

	const bool bWantsOverride = ( Params != nullptr && Params->bOverrideControlPitch );

	if ( bWantsOverride == false )
	{
		bControlPitchReached = false;
	}

	// 목표 각도에 닿은 뒤에는 플레이어에게 돌려준다. 어깨너머 조준처럼 진입 각도만 맞추면 되는 경우다.
	const bool bDriving = bWantsOverride && ( Params->bFreeControlPitchAfterReach == false || bControlPitchReached == false );

	// 복귀 보간 중에도 입력을 막는다. 열어두면 입력과 보간이 같은 값을 밀고 당겨 1도 이내 도달 판정이 나지 않는다.
	const bool bReturning = ( bWantsOverride == false && bIsControlPitchOverridden );
	bControlPitchLockRequested = bDriving || bReturning;

	// 평소에는 컨트롤 회전에 손대지 않는다. 마우스는 플레이어 것이다.
	if ( bWantsOverride == false && bIsControlPitchOverridden == false )
	{
		return;
	}

	if ( bWantsOverride && bIsControlPitchOverridden == false )
	{
		// 잠그기 시작하는 프레임. 지금 보고 있던 각도를 복귀 지점으로 기억한다.
		PreOverrideControlPitch = FRotator::NormalizeAxis(OwnerPlayerController->GetControlRotation().Pitch);
		bIsControlPitchOverridden = true;
	}

	if ( bWantsOverride && bDriving == false )
	{
		return;
	}

	const float TargetPitch = bWantsOverride ? Params->ControlPitch : PreOverrideControlPitch;

	FRotator ControlRotation = OwnerPlayerController->GetControlRotation();

	// NormalizeAxis 로 -180~180 으로 펴서 보간한다. GetControlRotation 은 0~360 이라 그대로 -55 를 향하면 305도를 한 바퀴 돈다.
	const float CurrentPitch = FRotator::NormalizeAxis(ControlRotation.Pitch);

	// Yaw 는 건드리지 않는다 — 부감에서도 배치 방향은 플레이어가 정해야 한다.
	ControlRotation.Pitch = FMath::FInterpTo(CurrentPitch, TargetPitch, DeltaTime, Speed);

	OwnerPlayerController->SetControlRotation(ControlRotation);

	if ( bWantsOverride && Params->bFreeControlPitchAfterReach && FMath::IsNearlyEqual(ControlRotation.Pitch, TargetPitch, 1.f) )
	{
		bControlPitchReached = true;
	}

	if ( bWantsOverride == false && FMath::IsNearlyEqual(ControlRotation.Pitch, PreOverrideControlPitch, 1.f) )
	{
		bIsControlPitchOverridden = false;

		// 같은 틱에 카메라가 정착 판정을 받으면 이 함수가 더는 불리지 않으니, 여기서 풀지 않으면 입력이 잠긴 채 굳는다.
		bControlPitchLockRequested = false;
	}
}

bool UYSCameraManageComponent::GetNormalizedCursorOffset(float DeadZone, FVector2D& OutOffset) const
{
	if ( OwnerPlayerController.IsValid() == false )
	{
		return false;
	}

	int32 SizeX = 0;
	int32 SizeY = 0;
	OwnerPlayerController->GetViewportSize(SizeX, SizeY);

	float MouseX = 0.f;
	float MouseY = 0.f;

	if ( SizeX <= 0 || SizeY <= 0 || OwnerPlayerController->GetMousePosition(MouseX, MouseY) == false )
	{
		return false;
	}

	FVector2D Normalized(
		( MouseX - SizeX * 0.5f ) / ( SizeX * 0.5f ),
		( MouseY - SizeY * 0.5f ) / ( SizeY * 0.5f ));

	const float Length = Normalized.Size();

	if ( Length <= DeadZone )
	{
		OutOffset = FVector2D::ZeroVector;
		return true;
	}

	// 데드존을 뺀 나머지를 다시 0~1 로 편다. 안 그러면 데드존 경계에서 값이 튄다.
	const float Scaled = FMath::Min(( Length - DeadZone ) / ( 1.f - DeadZone ), 1.f);

	OutOffset = Normalized / Length * Scaled;
	return true;
}

void UYSCameraManageComponent::TickCursorLean(const FYSCameraEffectParams* Params, float DeltaTime)
{
	USpringArmComponent* Boom = OwnerPlayer.IsValid() ? OwnerPlayer->GetCameraBoom() : nullptr;

	if ( IsValid(Boom) == false )
	{
		return;
	}

	FVector Target = FVector::ZeroVector;
	float Speed = RestoreInterpSpeed;

	FVector2D Cursor = FVector2D::ZeroVector;

	if ( Params != nullptr && Params->bUseCursorCameraLean && GetNormalizedCursorOffset(Params->CursorLeanDeadZone, Cursor) )
	{
		Speed = Params->CursorLeanInterpSpeed;

		// 요만 남긴 기저로 민다. 피치까지 섞으면 부감에서 카메라가 지면으로 파고든다.
		const FRotationMatrix Basis(FRotator( 0.f, OwnerPlayerController->GetControlRotation().Yaw, 0.f ));

		// 스크린 Y 는 아래로 증가한다. 커서가 위면 전방으로 밀어야 한다.
		Target = Basis.GetUnitAxis(EAxis::X) * ( -Cursor.Y ) * Params->CursorLeanDistance
			   + Basis.GetUnitAxis(EAxis::Y) * (  Cursor.X ) * Params->CursorLeanDistance;
	}

	Boom->TargetOffset = FMath::VInterpTo(Boom->TargetOffset, Target, DeltaTime, Speed);
}

void UYSCameraManageComponent::TickCameraEffect(float DeltaTime)
{
	const FYSCameraModeRequest* ActiveMode = ResolveActiveMode();

	// 스택이 비었고 이미 기본값에 도달했으면 할 일이 없다.
	if ( ActiveMode == nullptr && bCameraSettled )
	{
		return;
	}

	if ( OwnerPlayer.IsValid() == false )
	{
		return;
	}

	USpringArmComponent* Boom = OwnerPlayer->GetCameraBoom();
	UCameraComponent* Cam  = OwnerPlayer->GetFollowCamera();

	if ( !IsValid(Boom) || !IsValid(Cam) )
		return;

	const bool bHasMode = ( ActiveMode != nullptr );

	const float TargetArm   = bHasMode ? ActiveMode->Params.TargetArmLength : DefaultArmLength;
	const FVector TargetOff = bHasMode ? ActiveMode->Params.SocketOffset    : DefaultSocketOffset;
	const float TargetFOV   = bHasMode ? ActiveMode->Params.FieldOfView     : DefaultFOV;
	const FRotator Rotator  = bHasMode ? ActiveMode->Params.RelativeRotator : DefaultRotation;
	const float Speed = bHasMode ? ActiveMode->Params.InterpInSpeed   : RestoreInterpSpeed;

	Boom->TargetArmLength = FMath::FInterpTo(Boom->TargetArmLength, TargetArm,  DeltaTime, Speed);
	Boom->SocketOffset  = FMath::VInterpTo(Boom->SocketOffset,  TargetOff, DeltaTime, Speed);
	Cam->SetRelativeRotation(FMath::RInterpTo(Cam->GetRelativeRotation(), Rotator, DeltaTime, Speed));
	Cam->FieldOfView = FMath::FInterpTo(Cam->FieldOfView, TargetFOV, DeltaTime, Speed);
	bControlYawLockRequested = bHasMode && ActiveMode->Params.bLockControlYaw;

	// 요가 잠기면 컨트롤 회전은 상수가 된다. 캐릭터에 물려두면 조준 방향을 향하려는 회전과 매 틱 충돌한다.
	const bool bWantsControllerYaw = bHasMode ? ActiveMode->Params.bUseControllerRotationYaw : bControlYaw;

	OwnerPlayer->bUseControllerRotationYaw = bControlYawLockRequested ? false : bWantsControllerYaw;
	OwnerPlayer->GetCharacterMovement()->bOrientRotationToMovement = bHasMode ? ActiveMode->Params.bOrientToControlRotation : bOriginControl;

	TickControlPitch(bHasMode ? &ActiveMode->Params : nullptr, DeltaTime, Speed);
	TickCursorLean(bHasMode ? &ActiveMode->Params : nullptr, DeltaTime);

	if ( bHasMode == false )
	{
		const bool bArmDone = FMath::IsNearlyEqual(Boom->TargetArmLength, DefaultArmLength, 1.f);
		const bool bFOVDone = FMath::IsNearlyEqual(Cam->FieldOfView, DefaultFOV, 0.5f);
		const bool bLeanDone = Boom->TargetOffset.IsNearlyZero(1.f);

		// 피치 복귀가 남아 있으면 아직 정착이 아니다. 여기서 참이 되면 다음 틱부터 조기 반환에 걸려 부감으로 굳는다.
		if ( bArmDone && bFOVDone && bLeanDone && bIsControlPitchOverridden == false )
		{
			Boom->TargetArmLength = DefaultArmLength;
			Boom->SocketOffset  = DefaultSocketOffset;
			Boom->TargetOffset  = FVector::ZeroVector;
			Cam->FieldOfView      = DefaultFOV;
			Cam->SetRelativeRotation(DefaultRotation);
			bCameraSettled        = true;
		}
	}
}
