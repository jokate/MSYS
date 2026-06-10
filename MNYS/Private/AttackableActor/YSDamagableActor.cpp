// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackableActor/YSDamagableActor.h"


#include "AttackableActor/YSTraceObject.h"
#include "Library/YSBlueprintFunctionLibrary.h"

#if UE_EDITOR
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#endif

// Sets default values
AYSDamagableActor::AYSDamagableActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
#if WITH_EDITORONLY_DATA
	DebugBox = CreateEditorOnlyDefaultSubobject<UBoxComponent>(TEXT("DebugBox"));
	DebugSphere = CreateEditorOnlyDefaultSubobject<USphereComponent>(TEXT("DebugSphere"));
	DebugCapsule = CreateEditorOnlyDefaultSubobject<UCapsuleComponent>(TEXT("DebugCapsule"));

	for (UPrimitiveComponent* Comp : { (UPrimitiveComponent*)DebugBox, (UPrimitiveComponent*)DebugSphere, (UPrimitiveComponent*)DebugCapsule })
	{
		Comp->SetupAttachment(GetRootComponent());
		Comp->SetHiddenInGame(true);           // 런타임엔 완전히 숨김
		Comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
#endif
}


void AYSDamagableActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(TraceObject))
	{
		TraceObject->OnTraceHit.RemoveAll(this);
		TraceObject = nullptr;
	}
	
	Super::EndPlay(EndPlayReason);
}

void AYSDamagableActor::_OnHitCountDepleted()
{
	Destroy();
}

void AYSDamagableActor::OnActivate()
{
	TraceObject = UYSTraceObject::Create(this, this, OwnerActor.Get(), TraceConfig);
	TraceObject->OnHitCountDepleted.AddDynamic(this, &AYSDamagableActor::_OnHitCountDepleted);
	
	if ( IsValid(TraceObject) )
	{
		if (TraceConfig.bTraceOnce)
		{
			TraceObject->ExecuteOnce();
			Destroy();
		}	
	}
}

// Called every frame
void AYSDamagableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	_Trace(DeltaTime);
}

void AYSDamagableActor::_Trace(float DeltaTime)
{
	if (IsValid(TraceObject))
	{
		TraceObject->Tick(DeltaTime);
	}
}

#if UE_EDITOR
void AYSDamagableActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	_RefreshDebugVisualization();
}

void AYSDamagableActor::_RefreshDebugVisualization()
{
	// Shape에 따라 해당 컴포넌트만 보이게
	DebugBox->SetVisibility(TraceConfig.Shape == EYSTraceShape::Box);
	DebugSphere->SetVisibility(TraceConfig.Shape == EYSTraceShape::Sphere);
	DebugCapsule->SetVisibility(TraceConfig.Shape == EYSTraceShape::Capsule);

	// Extent 동기화
	DebugBox->SetBoxExtent(TraceConfig.Extent);
	DebugSphere->SetSphereRadius(TraceConfig.Extent.X);
	DebugCapsule->SetCapsuleSize(TraceConfig.Extent.X, TraceConfig.Extent.Y);
}

#endif
