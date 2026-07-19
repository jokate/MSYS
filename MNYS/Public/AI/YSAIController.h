// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "YSAIController.generated.h"

class UYSAIPerceptionComponent;
class UStateTreeAIComponent;

UCLASS()
class MNYS_API AYSAIController : public AAIController
{
	GENERATED_BODY()

public:
	static AYSAIController* Get(AActor* InActor);
	// Sets default values for this actor's properties
	AYSAIController();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void OnPossess(APawn* InPawn) override;
	void StartLogic() const;
	void StopLogic() const;
	
public : 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | Perception")
	TObjectPtr<UYSAIPerceptionComponent> MainPerceptionComponent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | State Tree")
	TObjectPtr<UStateTreeAIComponent> StateTreeComponent;
};
