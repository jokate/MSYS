// Fill out your copyright notice in the Description page of Project Settings.


#include "Targeting/YSSkillIndicator.h"

#include "Components/DecalComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

namespace YSIndicatorParams
{
	// 머티리얼 파라미터 이름. 머티리얼 쪽 이름을 바꾸면 여기도 같이 고쳐야 한다.
	static const FName ShapeType(TEXT("ShapeType"));
	static const FName ConeAngle(TEXT("ConeAngle"));
	static const FName Color(TEXT("Color"));
}

// 데칼을 아래로 투영시키는 로컬 회전.
// 이 상태에서 데칼의 로컬 Z축이 액터의 전방을 향하므로, DecalSize.Z 가 전방 길이가 된다.
static const FRotator YSIndicatorDecalRotation(-90.f, 0.f, 0.f);

AYSSkillIndicator::AYSSkillIndicator()
{
	// 조준 지점 산출과 갱신 호출은 타게팅 쪽이 한다. 이 액터는 스스로 돌 이유가 없다.
	PrimaryActorTick.bCanEverTick = false;

	// 로컬 전용 비주얼이다. 복제도 충돌도 필요 없다.
	bReplicates = false;
	SetCanBeDamaged(false);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	ShapeDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("ShapeDecal"));
	ShapeDecal->SetupAttachment(SceneRoot);
	ShapeDecal->SetRelativeRotation(YSIndicatorDecalRotation);

	// 사거리 링은 도형과 따로 움직인다. 부모를 따라가면 안 되므로 절대 트랜스폼으로 둔다.
	RangeDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("RangeDecal"));
	RangeDecal->SetupAttachment(SceneRoot);
	RangeDecal->SetUsingAbsoluteLocation(true);
	RangeDecal->SetUsingAbsoluteRotation(true);
	RangeDecal->SetWorldRotation(YSIndicatorDecalRotation);
	RangeDecal->SetVisibility(false);
}

AYSSkillIndicator* AYSSkillIndicator::CreateIndicator(const UObject* WorldContextObject, TSubclassOf<AYSSkillIndicator> IndicatorClass)
{
	if ( IsValid(WorldContextObject) == false || IndicatorClass == nullptr )
	{
		return nullptr;
	}

	UWorld* World = WorldContextObject->GetWorld();

	if ( IsValid(World) == false )
	{
		return nullptr;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.ObjectFlags |= RF_Transient;

	AYSSkillIndicator* Indicator = World->SpawnActor<AYSSkillIndicator>(IndicatorClass, FTransform::Identity, Params);

	if ( IsValid(Indicator) )
	{
		Indicator->Deactivate();
	}

	return Indicator;
}

void AYSSkillIndicator::BeginPlay()
{
	Super::BeginPlay();

	SetActorEnableCollision(false);

	EnsureDynamicMaterials();
	RefreshColor();
}

void AYSSkillIndicator::EnsureDynamicMaterials()
{
	if ( ShapeMID == nullptr && IsValid(ShapeDecal) && IsValid(ShapeMaterial) )
	{
		ShapeDecal->SetDecalMaterial(ShapeMaterial);
		ShapeMID = ShapeDecal->CreateDynamicMaterialInstance();
	}

	if ( RangeMID == nullptr && IsValid(RangeDecal) && IsValid(RangeMaterial) )
	{
		RangeDecal->SetDecalMaterial(RangeMaterial);
		RangeMID = RangeDecal->CreateDynamicMaterialInstance();
	}
}

void AYSSkillIndicator::ApplyShape(EYSIndicatorShapeType NewShape, const FVector& Origin, float Yaw)
{
	if ( IsValid(ShapeDecal) == false )
	{
		return;
	}

	// 머티리얼이 나중에 지정될 수도 있으므로 매번 확인한다. 이미 만들었으면 즉시 빠져나온다.
	EnsureDynamicMaterials();

	if ( IsHidden() )
	{
		SetActorHiddenInGame(false);
	}

	ShapeDecal->SetVisibility(true);

	// 도형의 기준점과 방향. 데칼은 이 액터를 부모로 두므로 액터를 옮기면 같이 따라온다.
	SetActorLocationAndRotation(Origin, FRotator(0.f, Yaw, 0.f));

	if ( CurrentShape == NewShape )
	{
		return;
	}

	CurrentShape = NewShape;

	if ( IsValid(ShapeMID) )
	{
		ShapeMID->SetScalarParameterValue(YSIndicatorParams::ShapeType, static_cast<float>(NewShape));
	}
}

void AYSSkillIndicator::SetShapeExtent(float ForwardOffset, float HalfWidth, float HalfLength)
{
	if ( IsValid(ShapeDecal) == false )
	{
		return;
	}

	// 부모(액터) 기준 X가 전방이다. 부채꼴·직선은 기준점이 도형의 시작점이므로 절반만큼 밀어준다.
	ShapeDecal->SetRelativeLocation(FVector(ForwardOffset, 0.f, 0.f));

	// 데칼 로컬 축 기준 — X는 투영 깊이, Y는 좌우 폭, Z는 전방 길이.
	ShapeDecal->DecalSize = FVector(ProjectionDepth, HalfWidth, HalfLength);
	ShapeDecal->MarkRenderStateDirty();
}

void AYSSkillIndicator::ShowCircle(const FVector& Center, float Radius)
{
	ApplyShape(EYSIndicatorShapeType::Circle, Center, 0.f);

	// 원형은 방향이 없다. 기준점이 곧 중심이라 전방 오프셋도 없다.
	SetShapeExtent(0.f, Radius, Radius);
}

void AYSSkillIndicator::ShowCone(const FVector& Origin, float Yaw, float Radius, float AngleDegrees)
{
	ApplyShape(EYSIndicatorShapeType::Cone, Origin, Yaw);

	// 꼭짓점이 Origin이고 전방으로 Radius 만큼 뻗는다. 좌우로는 최대 Radius 까지 벌어질 수 있다.
	SetShapeExtent(Radius * 0.5f, Radius, Radius * 0.5f);

	if ( IsValid(ShapeMID) )
	{
		ShapeMID->SetScalarParameterValue(YSIndicatorParams::ConeAngle, AngleDegrees);
	}
}

void AYSSkillIndicator::ShowLine(const FVector& Origin, float Yaw, float Length, float Width)
{
	ApplyShape(EYSIndicatorShapeType::Line, Origin, Yaw);

	// 시작점이 Origin이므로 데칼 중심을 전방으로 절반 밀어야 한다.
	SetShapeExtent(Length * 0.5f, Width * 0.5f, Length * 0.5f);
}

void AYSSkillIndicator::SetRangeRing(bool bShow, const FVector& Center, float Range)
{
	if ( IsValid(RangeDecal) == false )
	{
		return;
	}

	if ( bShow == false )
	{
		RangeDecal->SetVisibility(false);
		return;
	}

	EnsureDynamicMaterials();

	RangeDecal->SetWorldLocation(Center);
	RangeDecal->DecalSize = FVector(ProjectionDepth, Range, Range);
	RangeDecal->MarkRenderStateDirty();
	RangeDecal->SetVisibility(true);
}

void AYSSkillIndicator::SetValid(bool bInValid)
{
	if ( bIsValid == bInValid )
	{
		return;
	}

	bIsValid = bInValid;
	RefreshColor();
}

void AYSSkillIndicator::RefreshColor()
{
	const FLinearColor& Target = bIsValid ? ValidColor : InvalidColor;

	if ( IsValid(ShapeMID) )
	{
		ShapeMID->SetVectorParameterValue(YSIndicatorParams::Color, Target);
	}

	if ( IsValid(RangeMID) )
	{
		RangeMID->SetVectorParameterValue(YSIndicatorParams::Color, Target);
	}
}

void AYSSkillIndicator::Deactivate()
{
	CurrentShape = EYSIndicatorShapeType::None;

	if ( IsValid(ShapeDecal) )
	{
		ShapeDecal->SetVisibility(false);
	}

	if ( IsValid(RangeDecal) )
	{
		RangeDecal->SetVisibility(false);
	}

	SetActorHiddenInGame(true);

	// 다음 레디는 시전 가능 상태에서 시작한다. 직전 실패 색이 한 프레임 새어나오는 것을 막는다.
	bIsValid = true;
	RefreshColor();
}
