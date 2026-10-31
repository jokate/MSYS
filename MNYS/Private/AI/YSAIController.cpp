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

AYSAIController::AYSAIController()
{
	PrimaryActorTick.bCanEverTick = true;
	
	MainPerceptionComponent = CreateDefaultSubobject<UYSAIPerceptionComponent>("PerceptionComponent");
	StateTreeComponent = CreateDefaultSubobject<UStateTreeAIComponent>("StateTreeComponent");
	
}

void AYSAIController::BeginPlay()
{
	Super::BeginPlay();
}

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

