
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

void AYSDamagableActor::_OnHitCountDepleted()
{
	if (bNeedToRemainAfterHitCountDepleted)
	{
		return;
	}
	
	Destroy();
}

void AYSDamagableActor::_OnTraceHit(const TArray<FHitResult>& HitResults, const FName& DamageRow)
{
	if ( HitContext.IsValid() )
	{
		for ( const FHitResult& HitResult : HitResults)
		{
			HitContext->AddHitActor(HitResult.GetActor());
			HitContext->UpdateHitResult(TraceObject, HitResult);
		}
	}
}

void AYSDamagableActor::OnActivate_Implementation()
{
	Super::OnActivate_Implementation();
	TraceObject = UYSTraceObject::Create(this, this, InstigatorActor.Get(), TraceConfig);
	TraceObject->OnHitCountDepleted.AddDynamic(this, &AYSDamagableActor::_OnHitCountDepleted);
	TraceObject->OnTraceHit.AddDynamic(this, &AYSDamagableActor::_OnTraceHit);
	
	if ( IsValid(TraceObject) )
	{
		if (TraceConfig.bTraceOnce)
		{
			TraceObject->ExecuteOnce();
			Destroy();
		}	
	}
}

void AYSDamagableActor::SetPoolActive(bool bActive)
{
	if ( !bActive )
	{
		if (IsValid(TraceObject))
		{
			HitContext->RemoveTraceEntry(TraceObject);
			TraceObject->OnHitCountDepleted.RemoveAll(this);
			TraceObject->OnTraceHit.RemoveAll(this);
			TraceObject = nullptr;
		}
	}
	
	Super::SetPoolActive(bActive);
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

void AYSDamagableActor::ProcessTelegraph()
{
	TelegraphActor = AYSTelegraphActor::CreateTelegraph(
		this,
		TelegraphClass,
		GetActorTransform(),
		TraceConfig.Shape,
		TraceConfig.Extent,
		ActivationType == EYSAttackActivationType::TimeBased ? ActivateTime : 0.f);
	
	if ( IsValid(TelegraphActor) )
	{
		TelegraphActor->SetOwner(this);
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
	
	DebugBox->SetRelativeLocation(TraceConfig.RelativeLocation);
	DebugSphere->SetRelativeLocation(TraceConfig.RelativeLocation);
	DebugCapsule->SetRelativeLocation(TraceConfig.RelativeLocation);
}

#endif
