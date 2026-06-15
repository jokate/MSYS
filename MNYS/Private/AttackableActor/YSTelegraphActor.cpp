// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackableActor/YSTelegraphActor.h"

#include "Components/DecalComponent.h"


// Sets default values
AYSTelegraphActor::AYSTelegraphActor()
{
	PrimaryActorTick.bCanEverTick = true;
	FloorDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("Decal"));
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
		MID->SetScalarParameterValue(TEXT("ShapeType"), static_cast<float>(ShapeInfo));
		MID->SetScalarParameterValue(TEXT("Duration"), Duration);
		MID->SetScalarParameterValue(TEXT("StartTime"), GetWorld()->GetTimeSeconds());
	}
	
	FloorDecal->DecalSize = Extent;
	
	SetLifeSpan(Duration);
}

// Called every frame
void AYSTelegraphActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

