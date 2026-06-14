// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "YSAttackableBase.h"
#include "GameFramework/Actor.h"
#include "General/YSStruct.h"
#include "YSDamagableActor.generated.h"

class UCapsuleComponent;
class USphereComponent;
class UBoxComponent;
class UYSTraceObject;

UCLASS()
class MNYS_API AYSDamagableActor : public AYSAttackableBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AYSDamagableActor();
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	UFUNCTION()
	virtual void _OnHitCountDepleted();
	
	virtual void OnActivate_Implementation() override;
	
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
protected : 
	void _Trace(float DeltaTime);

protected : 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damagable")
	FYSTraceConfig TraceConfig;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damagable")
	bool bNeedToRemainAfterHitCountDepleted = false;
	
	UPROPERTY()
	TObjectPtr<UYSTraceObject> TraceObject;
	
	// AYSDamagableActor.h
#if WITH_EDITORONLY_DATA
protected:
	UPROPERTY(VisibleAnywhere, Category = "YS | Debug")
	TObjectPtr<UBoxComponent> DebugBox;

	UPROPERTY(VisibleAnywhere, Category = "YS | Debug")
	TObjectPtr<USphereComponent> DebugSphere;

	UPROPERTY(VisibleAnywhere, Category = "YS | Debug")
	TObjectPtr<UCapsuleComponent> DebugCapsule;
#endif

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	void _RefreshDebugVisualization();
#endif
};
