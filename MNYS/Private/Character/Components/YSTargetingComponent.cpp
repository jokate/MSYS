// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Components/YSTargetingComponent.h"

#include "NavigationSystem.h"
#include "Character/YSPlayerController.h"
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
		if ( UYSCameraManageComponent* PrevCamera = CurrentPlayerController->LockOnComponent ) 
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

	if ( UYSCameraManageComponent* Camera = CurrentPlayerController->LockOnComponent )
	{
		Camera->PushCameraMode(Requester, Spec.CameraParams, EYSCameraModePriority::Targeting);
	}
	
	// 조준점을 산출하기 전에 커서를 먼저 드러낸다.
	// 캡처 상태에서는 마우스 위치가 갱신되지 않아 첫 프레임이 화면 중앙으로 잡힌다.
	ApplyCursorMode(true);

	// 첫 프레임부터 올바른 도형이 뜨도록 한 번 즉시 산출해 반영한다.
	// 틱을 기다리면 이전 조준의 잔상이 한 프레임 보인다.
	CurrentResult = EvaluateTarget();
	RefreshIndicator();

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
	if ( UYSCameraManageComponent* Camera = CurrentPlayerController->LockOnComponent )
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

	RefreshIndicator();
}

void UYSTargetingComponent::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentPlayerController = Cast<AYSPlayerController>(GetOwner());
}

void UYSTargetingComponent::RefreshIndicator()
{
	if ( IsValid(Indicator) == false )
	{
		return;
	}

	// 도형 종류에 따라 그리는 방식과 사거리 링 표시 여부가 갈린다.
	// 원형·사각형은 조준점이 중심이고, 직선·부채꼴·자기중심원은 시전자가 기준이다.
	if ( const FYSTargetingShape* Shape = CurrentSpec.Shape.GetPtr<FYSTargetingShape>() )
	{
		Shape->Draw(Indicator, CurrentResult, GetCasterLocation());
	}

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
	const FYSTargetingShape* Shape = CurrentSpec.Shape.GetPtr<FYSTargetingShape>();

	if ( Shape == nullptr )
	{
		Result.Location  = Caster;
		Result.Direction = GetCasterForward();
		return Result;
	}

	FVector Desired = FVector::ZeroVector;
	if ( ProjectScreenToGround(Desired) == false )
	{
		Result.Location  = Caster;
		Result.Direction = GetCasterForward();
		return Result;
	}

	// 방향은 도형 종류와 무관하게 항상 산출한다. 직선·부채꼴은 이 값만 쓴다.
	FVector Offset = Desired - Caster;
	Offset.Z = 0.f;
	Result.Direction = Offset.IsNearlyZero() ? GetCasterForward() : Offset.GetSafeNormal();

	// 시전자 기준 도형은 조준점을 위치로 쓰지 않는다. 사거리 판정도 의미가 없다.
	if ( Shape->UsesAimPointAsOrigin() == false )
	{
		Result.Location = Caster;
		Result.bValid   = true;
		return Result;
	}

	// 사거리는 수평 거리로만 잰다.
	// 높이차를 포함시키면 언덕 위 적이 사거리 밖으로 밀려난다.
	const float Range = Shape->GetRange();
	const bool bOutOfRange = ( Range > 0.f ) && ( Offset.SizeSquared() > FMath::Square(Range) );

	if ( bOutOfRange && Shape->bClampToRange )
	{
		// XY를 당겼으니 Z를 다시 지면에 붙인다.
		Result.Location = SnapToGround(Caster + Offset.GetSafeNormal() * Range);
	}
	else
	{
		Result.Location = Desired;
	}

	// 클램프를 켰으면 항상 사거리 안이므로 유효하다.
	Result.bValid = Shape->bClampToRange || ( bOutOfRange == false );

	return Result;
}

bool UYSTargetingComponent::GetAimScreenPosition(FVector2D& OutScreenPosition) const
{
	if ( IsValid(CurrentPlayerController) == false )
	{
		return false;
	}

	int32 SizeX = 0;
	int32 SizeY = 0;
	CurrentPlayerController->GetViewportSize(SizeX, SizeY);

	if ( SizeX <= 0 || SizeY <= 0 )
	{
		return false;
	}

	if ( AimSource == EYSAimSource::MouseCursor )
	{
		float MouseX = 0.f;
		float MouseY = 0.f;

		// 커서가 뷰포트 밖이거나 패드로 조작 중이면 실패한다. 이때는 아래 중앙 폴백으로 넘어간다.
		if ( CurrentPlayerController->GetMousePosition(MouseX, MouseY) )
		{
			OutScreenPosition = FVector2D(MouseX, MouseY);
			return true;
		}
	}

	OutScreenPosition = FVector2D(SizeX * 0.5f, SizeY * 0.5f);
	return true;
}

bool UYSTargetingComponent::ProjectScreenToGround(FVector& OutLocation) const
{

	UWorld* World = GetWorld();

	if ( IsValid(CurrentPlayerController) == false || IsValid(World) == false )
	{
		return false;
	}

	FVector2D ScreenPosition = FVector2D::ZeroVector;

	if ( GetAimScreenPosition(ScreenPosition) == false )
	{
		return false;
	}

	FVector WorldOrigin    = FVector::ZeroVector;
	FVector WorldDirection = FVector::ForwardVector;

	if ( CurrentPlayerController->DeprojectScreenPositionToWorld(ScreenPosition.X, ScreenPosition.Y, WorldOrigin, WorldDirection) == false )
	{
		return false;
	}

	const FVector TraceEnd = WorldOrigin + WorldDirection * MaxTraceDistance;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(YSTargeting), false);
	Params.AddIgnoredActor(CurrentPlayerController->GetPawn());

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
		if (NavSystem->ProjectPointToNavigation(Location, NavLocation, FVector(0.f, 0.f, YS_PROJECTION_MAX_DISTANCE)))
		{
			return NavLocation.Location;
		}
	}

	return Location;
}

FVector UYSTargetingComponent::GetCasterLocation() const
{
	const APawn* Pawn = IsValid(CurrentPlayerController) ? CurrentPlayerController->GetPawn() : nullptr;

	return IsValid(Pawn) ? Pawn->GetActorLocation() : FVector::ZeroVector;
}

FVector UYSTargetingComponent::GetCasterForward() const
{
	const APawn* Pawn = IsValid(CurrentPlayerController) ? CurrentPlayerController->GetPawn() : nullptr;

	return IsValid(Pawn) ? Pawn->GetActorForwardVector() : FVector::ForwardVector;
}


void UYSTargetingComponent::ApplyCursorMode(bool bTargeting)
{
	if ( IsValid(CurrentPlayerController) == false )
	{
		return;
	}

	// 조준이 연달아 시작되면 이미 바꿔둔 값을 원본으로 저장해버린다. 한 번만 처리한다.
	if ( bTargeting == bCursorModeApplied )
	{
		return;
	}

	bCursorModeApplied = bTargeting;
}

void UYSTargetingComponent::EndTargetingInternal()
{
	ApplyCursorMode(false);

	CurrentRequester = nullptr;
	CurrentResult = FYSTargetingResult();

	// 도형도 비운다. 남겨두면 다음 레디가 스펙을 덮기 전에 옛 도형이 한 프레임 그려진다.
	CurrentSpec.Shape.Reset();

	if ( IsValid(Indicator) )
	{
		Indicator->Deactivate();
	}

	SetComponentTickEnabled(false);
}

