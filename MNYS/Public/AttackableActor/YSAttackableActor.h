// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "General/YSStruct.h"
#include "YSAttackableActor.generated.h"

class UYSTraceObject;

UCLASS()
class MNYS_API AYSAttackableActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AYSAttackableActor();
	virtual void AllocateInstigator(AActor* InInstigator);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	virtual void _OnHitCountDepleted();
	
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
protected : 
	void _Trace(float DeltaTime);

	UFUNCTION()
	void _OnTraceObjectHit(const TArray<FHitResult>& HitResults, const FName& DamageRow);
protected : 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Root")
	TObjectPtr<USceneComponent> SceneRoot;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
	TObjectPtr<UStaticMeshComponent> RootMesh;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	FYSTraceConfig TraceConfig;
	
	UPROPERTY()
	TObjectPtr<UYSTraceObject> TraceObject;
	
	UPROPERTY()
	TWeakObjectPtr<AActor> OwnerActor;
};
