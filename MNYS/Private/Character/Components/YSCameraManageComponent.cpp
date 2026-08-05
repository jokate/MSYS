// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Components/YSCameraManageComponent.h"

#include "YSAbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Character/YSCharacterPlayer.h"
#include "Character/YSPlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "General/YSGameplayTag.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Library/YSBlueprintFunctionLibrary.h"


// Sets default values for this component's properties
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


// Called when the game starts
void UYSCameraManageComponent::BeginPlay()
{
	Super::BeginPlay();

	AYSPlayerController* PlayerController = Cast<AYSPlayerController>(GetOwner());

	if ( IsValid(PlayerController) )
	{
		OwnerPlayer = Cast<AYSCharacterPlayer>(PlayerController->GetPawn());
		OwnerPlayerController = PlayerController;

		// 스쿼드 태그 교대는 UnPossess -> Possess 로 폰을 갈아끼운다.
		// 여기서 다시 잡아주지 않으면 벤치로 내려간 이전 폰의 카메라를 계속 만지게 된다.
		PlayerController->OnPossessedPawnChanged.AddDynamic(this, &UYSCameraManageComponent::HandlePossessedPawnChanged);
	}

	CaptureCameraDefaults();
}

void UYSCameraManageComponent::HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	OwnerPlayer = Cast<AYSCharacterPlayer>(NewPawn);

	// 새 캐릭터의 기본값을 복귀 지점으로 다시 잡는다.
	// 캐릭터마다 암 길이·FOV가 다르므로 이전 값으로 복귀하면 엉뚱한 화각이 된다.
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
		bOriginControl = OwnerPlayer->bUseControllerRotationYaw;
	}
}


// Called every frame
void UYSCameraManageComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ProcessLockOnFunction(DeltaTime);
	TickCameraEffect(DeltaTime);
}

void UYSCameraManageComponent::ForceSetLockOn(AActor* TargetToLockOn)
{
	CurrentLockedTarget = TargetToLockOn;

	if ( OwnerPlayer.IsValid() == false )
	{
		return;
	}

	FRotator CharacterRotation = UYSBlueprintFunctionLibrary::GetEventRotation(EYSDirectionPolicy::UseTowardLockOnTarget, OwnerPlayer.Get(), NAME_None, FRotator::ZeroRotator);
	OwnerPlayer->SetActorRotation(CharacterRotation);

	// 락온도 카메라 모드의 요청자 중 하나다. 조준·연계기가 위에 얹히면 그쪽이 이기고,
	// 그것들이 빠지면 다시 락온 프로파일로 돌아온다.
	if ( bUseLockOnCameraMode )
	{
		PushCameraMode(this, LockOnCameraParams, EYSCameraModePriority::LockOn);
	}
}

void UYSCameraManageComponent::ReleaseLockOn()
{
	CurrentLockedTarget = nullptr;

	PopCameraMode(this);
}

void UYSCameraManageComponent::ProcessLockOnFunction(float DeltaTime)
{
	if ( OwnerPlayer.IsValid() == false )
		return;

	if ( CurrentLockedTarget.IsValid() )
	{
		ChaseCamera(DeltaTime);
		return;
	}
}

void UYSCameraManageComponent::ChaseCamera(float DeltaTime)
{
	if ( OwnerPlayerController.IsValid() == false || CurrentLockedTarget.IsValid() == false )
		return;

	FVector ToTarget = (CurrentLockedTarget->GetActorLocation() - OwnerPlayer->GetActorLocation()).GetSafeNormal();
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

	// 복귀 속도는 빠지는 요청의 값을 쓴다.
	// 남은 요청이 있으면 그쪽으로 블렌드하므로 아래에서 다시 덮인다.
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

const FYSCameraModeRequest* UYSCameraManageComponent::ResolveActiveMode()
{
	const FYSCameraModeRequest* Best = nullptr;

	for ( int32 Index = CameraModeStack.Num() - 1; Index >= 0; --Index )
	{
		// 요청자가 사라졌으면 요청도 무효다.
		// EndAbility를 못 거치고 파괴된 어빌리티가 카메라를 물고 늘어지는 것을 막는다.
		if ( CameraModeStack[Index].Requester.IsValid() == false )
		{
			CameraModeStack.RemoveAt(Index);
			bCameraSettled = false;
			continue;
		}

		// 뒤에서부터 순회하므로, 우선순위가 같으면 나중에 들어온 요청이 남는다.
		if ( Best == nullptr || CameraModeStack[Index].Priority > Best->Priority )
		{
			Best = &CameraModeStack[Index];
		}
	}

	return Best;
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
	OwnerPlayer->bUseControllerRotationYaw = bHasMode ? ActiveMode->Params.bOrientToControlRotation : bOriginControl;

	if ( bHasMode == false )
	{
		const bool bArmDone = FMath::IsNearlyEqual(Boom->TargetArmLength, DefaultArmLength, 1.f);
		const bool bFOVDone = FMath::IsNearlyEqual(Cam->FieldOfView, DefaultFOV, 0.5f);

		if ( bArmDone && bFOVDone )
		{
			Boom->TargetArmLength = DefaultArmLength;
			Boom->SocketOffset  = DefaultSocketOffset;
			Cam->FieldOfView      = DefaultFOV;
			Cam->SetRelativeRotation(DefaultRotation);
			bCameraSettled        = true;
		}
	}
}
