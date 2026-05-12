// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "YSCharacterBase.h"
#include "YSCharacter_Enemy.generated.h"

UCLASS()
class MNYS_API AYSCharacter_Enemy : public AYSCharacterBase
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AYSCharacter_Enemy(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
};
