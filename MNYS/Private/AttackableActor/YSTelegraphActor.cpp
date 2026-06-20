// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackableActor/YSTelegraphActor.h"

#include "Components/DecalComponent.h"


// Sets default values
AYSTelegraphActor::AYSTelegraphActor()
{
	PrimaryActorTick.bCanEverTick = true;
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
	
	FloorDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("Decal"));
	FloorDecal->SetupAttachment(SceneRoot);
	FloorDecal->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
}

AYSTelegraphActor* AYSTelegraphActor::CreateTelegraph(const UObject* WorldContextObject, TSubclassOf<AYSTelegraphActor> ActorClass, 
	const FTransform& Transform,
	EYSTraceShape InShapeInfo, const FVector& InExtent, float InDuration)
{
	if ( IsValid(WorldContextObject) == false || ActorClass == nullptr )
	{
		return nullptr;
	}
	
	UWorld* World = WorldContextObject->GetWorld();
	
	if (IsValid(World) == false )
	{
		return nullptr;
	}
	
	AYSTelegraphActor* TelegraphActor = World->SpawnActorDeferred<AYSTelegraphActor>(ActorClass, Transform);
	TelegraphActor->ShapeInfo = InShapeInfo;
	TelegraphActor->Extent = InExtent;
	TelegraphActor->Duration = InDuration;
	
	TelegraphActor->FinishSpawning(Transform);
	return TelegraphActor;
}

// Called when the game starts or when spawned
void AYSTelegraphActor::BeginPlay()
{
	Super::BeginPlay();
	
	UMaterialInstanceDynamic* MID = FloorDecal->CreateDynamicMaterialInstance();
	
	if ( IsValid(MID) == true )
	{
		MID->SetScalarParameterValue(TEXT("ShapeType"), ShapeInfo == EYSTraceShape::Box ? 0 : 1);
		MID->SetScalarParameterValue(TEXT("Duration"), Duration);
		MID->SetScalarParameterValue(TEXT("StartTime"), GetWorld()->GetTimeSeconds());
	}
	
	
	if ( ShapeInfo == EYSTraceShape::Box )
	{
		FloorDecal->DecalSize = FloorDecal->GetComponentRotation().RotateVector(Extent);
	}
	else
	{
		FloorDecal->DecalSize = FVector(Extent.X, Extent.X, Extent.X);
	}
	
	SetLifeSpan(Duration);
}

// Called every frame
void AYSTelegraphActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

