// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackableActor/YSAttackableActor.h"

#include "AttackableActor/YSTraceObject.h"
#include "Library/YSBlueprintFunctionLibrary.h"

#if UE_EDITOR
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#endif

// Sets default values
AYSAttackableActor::AYSAttackableActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
	
	RootMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RootMesh"));
	
	RootMesh->SetupAttachment(SceneRoot);

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

void AYSAttackableActor::AllocateInstigator(AActor* InInstigator)
{
	OwnerActor = InInstigator;
	TraceObject = UYSTraceObject::Create(this, this, InInstigator, TraceConfig);
	TraceObject->OnTraceHit.AddDynamic(this, &AYSAttackableActor::_OnTraceObjectHit);
	TraceObject->OnHitCountDepleted.AddDynamic(this, &AYSAttackableActor::_OnHitCountDepleted);
}

// Called when the game starts or when spawned
void AYSAttackableActor::BeginPlay()
{
	Super::BeginPlay();
	
	if ( IsValid(TraceObject) )
	{
		if (TraceConfig.bTraceOnce)
		{
			TraceObject->ExecuteOnce();
			Destroy();
		}	
	}
}

void AYSAttackableActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(TraceObject))
	{
		TraceObject->OnTraceHit.RemoveAll(this);
		TraceObject = nullptr;
	}
	
	Super::EndPlay(EndPlayReason);
}

void AYSAttackableActor::_OnHitCountDepleted()
{
	Destroy();
}

// Called every frame
void AYSAttackableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	_Trace(DeltaTime);
}

void AYSAttackableActor::_Trace(float DeltaTime)
{
	if (IsValid(TraceObject))
	{
		TraceObject->Tick(DeltaTime);
	}
}

void AYSAttackableActor::_OnTraceObjectHit(const TArray<FHitResult>& HitResults, const FName& DamageRow)
{
	UYSBlueprintFunctionLibrary::ProcessHits(
		OwnerActor.Get(),
		HitResults,
		DamageRow,
		[this](const TArray<FHitResult>& ValidHits) { _DecreaseValidHits(ValidHits); }
	);
}

void AYSAttackableActor::_DecreaseValidHits(const TArray<FHitResult>& HitResults)
{
	if (IsValid(TraceObject))
	{
		for (int32 i = 0; i < HitResults.Num(); ++i)
		{
			TraceObject->DecreaseHitProcessCount();
		}
	}
}

#if UE_EDITOR
void AYSAttackableActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	_RefreshDebugVisualization();
}

void AYSAttackableActor::_RefreshDebugVisualization()
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
