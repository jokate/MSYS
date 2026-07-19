// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/YSAIController.h"

#include "AI/Component/YSAIPerceptionComponent.h"
#include "Components/StateTreeAIComponent.h"
#include "General/YSStruct.h"


AYSAIController* AYSAIController::Get(AActor* InActor)
{
	AController* Controller = InActor->GetInstigatorController();
	
	if ( IsValid(Controller) == false)
	{
		return nullptr;
	}
	
	return Cast<AYSAIController>(Controller);
}

// Sets default values
AYSAIController::AYSAIController()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	MainPerceptionComponent = CreateDefaultSubobject<UYSAIPerceptionComponent>("PerceptionComponent");
	StateTreeComponent = CreateDefaultSubobject<UStateTreeAIComponent>("StateTreeComponent");
	
}

// Called when the game starts or when spawned
void AYSAIController::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AYSAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AYSAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	MainPerceptionComponent->SetTargetActorCollections(MakeShared<FYSTargetingActorCollections>(InPawn));
	StartLogic();
}

void AYSAIController::StartLogic() const
{
	StateTreeComponent->StartLogic();
}

void AYSAIController::StopLogic() const
{
	StateTreeComponent->StopLogic(TEXT("Stopped"));
}

