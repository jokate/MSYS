// Fill out your copyright notice in the Description page of Project Settings.


#include "Stage/YSZoneSlot.h"

#include "Components/BoxComponent.h"

AYSZoneSlot::AYSZoneSlot()
{
	PrimaryActorTick.bCanEverTick = false;

	Bounds = CreateDefaultSubobject<UBoxComponent>(TEXT("Bounds"));
	Bounds->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Bounds->SetBoxExtent(FVector(2000.f, 2000.f, 500.f));
	Bounds->SetHiddenInGame(true);
	RootComponent = Bounds;
}

float AYSZoneSlot::GetDistanceTo(const FVector& Location) const
{
	// 회전한 자리도 상자 기준으로 재려고 자리 공간으로 옮겨 잰다.
	const FVector Local = Bounds->GetComponentTransform().InverseTransformPositionNoScale(Location);
	const FVector Extent = Bounds->GetScaledBoxExtent();
	return FMath::Sqrt(FBox(-Extent, Extent).ComputeSquaredDistanceToPoint(Local));
}
