
// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackableActor/YSDamagableActor.h"


#include "Ability/YSGameplayAbility.h"
#include "AttackableActor/YSTelegraphActor.h"
#include "AttackableActor/YSTraceObject.h"
#include "Library/YSBlueprintFunctionLibrary.h"

#if UE_EDITOR
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#endif

AYSDamagableActor::AYSDamagableActor()
{
	PrimaryActorTick.bCanEverTick = true;
	
#if WITH_EDITORONLY_DATA
	DebugBox = CreateEditorOnlyDefaultSubobject<UBoxComponent>(TEXT("DebugBox"));
	DebugSphere = CreateEditorOnlyDefaultSubobject<USphereComponent>(TEXT("DebugSphere"));
	DebugCapsule = CreateEditorOnlyDefaultSubobject<UCapsuleComponent>(TEXT("DebugCapsule"));

	for (UPrimitiveComponent* Comp : { (UPrimitiveComponent*)DebugBox, (UPrimitiveComponent*)DebugSphere, (UPrimitiveComponent*)DebugCapsule })
	{
		Comp->SetupAttachment(GetRootComponent());
		Comp->SetHiddenInGame(true);
		Comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
#endif
}

void AYSDamagableActor::BeginPlay()
{
	Super::BeginPlay();
#if WITH_EDITORONLY_DATA
	if ( IsValid(DebugBox) && IsValid(DebugSphere) && IsValid(DebugCapsule) )
	{
		DebugBox->DestroyComponent();
		DebugSphere->DestroyComponent();
		DebugCapsule->DestroyComponent();
	}
#endif
}


void AYSDamagableActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AYSDamagableActor::_OnTraceEnded()
{
	if ( TraceConfig.bTraceOnce == false && bNeedToRemainAfterHitCountDepleted )
	{
		return;
	}
	
	ReturnToPoolAfter(0.f);
}

void AYSDamagableActor::OnActivate_Implementation()
{
	Super::OnActivate_Implementation();
	TraceObject = UYSTraceObject::Create(this, this, InstigatorActor.Get(), TraceConfig, EffectScale, HitContext);
	TraceObject->OnTraceEnded.AddDynamic(this, &AYSDamagableActor::_OnTraceEnded);
	TraceObject->Start();
}

void AYSDamagableActor::SetPoolActive(bool bActive)
{
	if ( !bActive && IsValid(TraceObject) )
	{
		TraceObject->Stop();
		TraceObject = nullptr;
	}
	
	Super::SetPoolActive(bActive);
}

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

AYSTelegraphActor* AYSDamagableActor::SpawnTelegraph(UWorld* World, const FTransform& Transform, float Duration) const
{
	return AYSTelegraphActor::CreateTelegraph(World, TelegraphClass, Transform, TraceConfig.Shape, TraceConfig.Extent, Duration);
}

#if UE_EDITOR
void AYSDamagableActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	_RefreshDebugVisualization();
}

void AYSDamagableActor::_RefreshDebugVisualization()
{
	DebugBox->SetVisibility(TraceConfig.Shape == EYSTraceShape::Box);
	DebugSphere->SetVisibility(TraceConfig.Shape == EYSTraceShape::Sphere);
	DebugCapsule->SetVisibility(TraceConfig.Shape == EYSTraceShape::Capsule);

	DebugBox->SetBoxExtent(TraceConfig.Extent);
	DebugSphere->SetSphereRadius(TraceConfig.Extent.X);
	DebugCapsule->SetCapsuleSize(TraceConfig.Extent.X, TraceConfig.Extent.Y);
	
	DebugBox->SetRelativeLocation(TraceConfig.RelativeLocation);
	DebugSphere->SetRelativeLocation(TraceConfig.RelativeLocation);
	DebugCapsule->SetRelativeLocation(TraceConfig.RelativeLocation);
}

#endif
