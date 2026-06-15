// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "General/YSEnum.h"
#include "YSTelegraphActor.generated.h"

enum class EYSTraceShape : uint8;
struct FYSTraceConfig;

UCLASS()
class MNYS_API AYSTelegraphActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AYSTelegraphActor();

	static AYSTelegraphActor* CreateTelegraph(const UObject* WorldContextObject, TSubclassOf<AYSTelegraphActor> ActorClass,
		const FTransform& Transform, EYSTraceShape InShapeInfo, const FVector& InExtent, float InDuration );
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
public : 
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UDecalComponent> FloorDecal;
	
private : 
	EYSTraceShape ShapeInfo = EYSTraceShape::Box;
	FVector Extent;
	float Duration;
};
