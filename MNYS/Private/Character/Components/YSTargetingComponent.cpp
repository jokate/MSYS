// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Components/YSTargetingComponent.h"

#include "NavigationSystem.h"
#include "Character/Components/YSCameraManageComponent.h"
#include "General/YSDefine.h"
#include "Targeting/YSSkillIndicator.h"


// Sets default values for this component's properties
UYSTargetingComponent::UYSTargetingComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

UYSTargetingComponent* UYSTargetingComponent::Get(const AActor* Actor)
{
	if ( IsValid(Actor) == false )
	{
		return nullptr;
	}
	
	AController* Controller = Actor->GetInstigatorController();
	if ( IsValid(Controller) == false )
	{
		return nullptr;
	}

	return Controller->FindComponentByClass<UYSTargetingComponent>();
}

bool UYSTargetingComponent::BeginTargeting(UObject* Requester, const FYSTargetingSpec& Spec)
{
	if ( IsValid(Requester) == false )
	{
		return false;
	}

	// 다른 스킬이 조준 중이었다면 그쪽 카메라 요청부터 회수한다.
	// 안 그러면 밀려난 요청이 스택에 남아 조준이 끝나도 카메라가 돌아오지 않는다.
	if ( CurrentRequester.IsValid() && CurrentRequester.Get() != Requester )
	{
		if ( UYSCameraManageComponent* PrevCamera = UYSCameraManageComponent::Get(GetOwner()) )
		{
			PrevCamera->PopCameraMode(CurrentRequester.Get());
		}
	}

	if ( IsValid(Indicator) == false )
	{
		Indicator = AYSSkillIndicator::CreateIndicator(this, IndicatorClass);

		if ( IsValid(Indicator) == false )
		{
			return false;
		}
	}

	CurrentRequester = Requester;
	CurrentSpec = Spec;

	if ( UYSCameraManageComponent* Camera = UYSCameraManageComponent::Get(GetOwner()) )
	{
		Camera->PushCameraMode(Requester, Spec.CameraParams, EYSCameraModePriority::Targeting);
	}
	
	CurrentResult = EvaluateTarget();
	SetComponentTickEnabled(true);
	return true;
}

void UYSTargetingComponent::EndTargeting(UObject* Requester)
{
	// 현재 요청한 애가 이놈이 아니면 가라..
	if ( CurrentRequester.Get() != Requester )
	{
		return;
	}
	
	// 카메라를 빼준다. 
	if ( UYSCameraManageComponent* Camera = UYSCameraManageComponent::Get(GetOwner()) )
	{
		Camera->PopCameraMode(Requester);
	}

	// 인디케이터 비활.
	if ( IsValid(Indicator) )
	{
		Indicator->Deactivate();
	}
	
	EndTargetingInternal();
}


// Called every frame
void UYSTargetingComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                          FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if ( CurrentRequester.IsValid() == false )
	{
		EndTargetingInternal();
		return;
	}

	CurrentResult = EvaluateTarget();

	if ( IsValid(Indicator) == false )
	{
		return;
	}

	Indicator->ShowCircle(CurrentResult.Location, CurrentSpec.Radius);
	Indicator->SetRangeRing(true, GetCasterLocation(), CurrentSpec.Range);
	Indicator->SetValid(CurrentResult.bValid);
}

void UYSTargetingComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if ( IsValid(Indicator) )
	{
		Indicator->Destroy();
		Indicator = nullptr;
	}
	
	Super::EndPlay(EndPlayReason);
}

FYSTargetingResult UYSTargetingComponent::EvaluateTarget() const
{
	FYSTargetingResult Result;

	const FVector Caster = GetCasterLocation();

	FVector Desired = FVector::ZeroVector;
	if ( ProjectScreenToGround(Desired) == false )
	{
		Result.Location  = Caster;
		Result.Direction = GetCasterForward();
		return Result;
	}
	
	FVector Offset = Desired - Caster;
	Offset.Z = 0.f;

	const bool bOutOfRange = Offset.SizeSquared() > FMath::Square(CurrentSpec.Range);

	if ( bOutOfRange && CurrentSpec.bClampToRange )
	{
		// XY를 당겼으니 Z를 다시 지면에 붙인다.
		Result.Location = SnapToGround(Caster + Offset.GetSafeNormal() * CurrentSpec.Range);
		Offset = Result.Location - Caster;
		Offset.Z = 0.f;
	}
	else
	{
		Result.Location = Desired;
	}

	Result.Direction = Offset.IsNearlyZero() ? GetCasterForward() : Offset.GetSafeNormal();

	// 클램프를 켰으면 항상 사거리 안이므로 유효하다.
	Result.bValid = CurrentSpec.bClampToRange || ( bOutOfRange == false );

	return Result;
}

bool UYSTargetingComponent::ProjectScreenToGround(FVector& OutLocation) const
{
	APlayerController* PC = Cast<APlayerController>(GetOwner());
	UWorld* World = GetWorld();

	if ( IsValid(PC) == false || IsValid(World) == false )
	{
		return false;
	}

	int32 SizeX = 0;
	int32 SizeY = 0;
	PC->GetViewportSize(SizeX, SizeY);

	if ( SizeX <= 0 || SizeY <= 0 )
	{
		return false;
	}
	
	const float ScreenX = SizeX * 0.5f;
	const float ScreenY = SizeY * 0.5f;

	FVector WorldOrigin    = FVector::ZeroVector;
	FVector WorldDirection = FVector::ForwardVector;

	if ( PC->DeprojectScreenPositionToWorld(ScreenX, ScreenY, WorldOrigin, WorldDirection) == false )
	{
		return false;
	}

	const FVector TraceEnd = WorldOrigin + WorldDirection * MaxTraceDistance;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(YSTargeting), false);
	Params.AddIgnoredActor(PC->GetPawn());

	FHitResult Hit;
	if ( World->LineTraceSingleByChannel(Hit, WorldOrigin, TraceEnd, TraceChannel, Params) )
	{
		OutLocation = Hit.ImpactPoint;
		return true;
	}

	// 하늘이나 낭떠러지를 겨눈 경우.
	// 폴백이 없으면 카메라를 위로 드는 순간 인디케이터가 사라진다.
	// 시전자 발밑 높이의 수평면과 시선을 교차시켜 지점을 만든다.
	if ( FMath::IsNearlyZero(WorldDirection.Z) )
	{
		return false;
	}

	// 수평 형태의 바닥 기준으로..
	OutLocation = FMath::LinePlaneIntersection(WorldOrigin, TraceEnd, GetCasterLocation(), FVector::UpVector);
	return true;
}

FVector UYSTargetingComponent::SnapToGround(const FVector& Location) const
{
	UWorld* World = GetWorld();

	if ( IsValid(World) == false )
	{
		return Location;
	}

	// 지면에 붙이는 정책이 켜져있으면 NavMesh에 붙인다. NavMesh가 없으면 그냥 원래 위치를 반환한다~
	if (UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(World))
	{
		FNavLocation NavLocation;
		if (NavSystem->ProjectPointToNavigation(Location, NavLocation, FVector(YS_PROJECTION_MAX_DISTANCE)))
		{
			return NavLocation.Location;
		}
	}

	return Location;
}

FVector UYSTargetingComponent::GetCasterLocation() const
{
	const APlayerController* PC = Cast<APlayerController>(GetOwner());
	const APawn* Pawn = IsValid(PC) ? PC->GetPawn() : nullptr;

	return IsValid(Pawn) ? Pawn->GetActorLocation() : FVector::ZeroVector;
}

FVector UYSTargetingComponent::GetCasterForward() const
{
	const APlayerController* PC = Cast<APlayerController>(GetOwner());
	const APawn* Pawn = IsValid(PC) ? PC->GetPawn() : nullptr;

	return IsValid(Pawn) ? Pawn->GetActorForwardVector() : FVector::ForwardVector;
}


void UYSTargetingComponent::EndTargetingInternal()
{
	CurrentRequester = nullptr;
	CurrentResult = FYSTargetingResult();

	if ( IsValid(Indicator) )
	{
		Indicator->Deactivate();
	}

	SetComponentTickEnabled(false);
}

