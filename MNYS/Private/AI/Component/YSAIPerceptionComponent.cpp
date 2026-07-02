// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Component/YSAIPerceptionComponent.h"

#include "AI/YSAIController.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Sight.h"


// Sets default values for this component's properties
UYSAIPerceptionComponent::UYSAIPerceptionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

TSharedPtr<FYSTargetingActorCollections> UYSAIPerceptionComponent::GetTargetingCollection(AActor* Actor)
{
	if (IsValid(Actor) == false )
	{
		return nullptr;
	}
	
	AYSAIController* AIController = Actor->GetInstigatorController<AYSAIController>();
	
	if (IsValid(AIController) == false )
	{
		return nullptr;
	}
	
	return AIController->MainPerceptionComponent->GetTargetingActorCollections();
}


// Called when the game starts
void UYSAIPerceptionComponent::BeginPlay()
{
	Super::BeginPlay();

	OnTargetPerceptionUpdated.AddDynamic(this, &UYSAIPerceptionComponent::OnPerceptionUpdated_Callback);
	OnTargetPerceptionForgotten.AddDynamic(this, &UYSAIPerceptionComponent::OnActorForgotten);
}

void UYSAIPerceptionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	TargetingActorCollections = nullptr;
	Super::EndPlay(EndPlayReason);
}


// Called every frame
void UYSAIPerceptionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                             FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}


void UYSAIPerceptionComponent::OnPerceptionUpdated_Callback(AActor* Actor, FAIStimulus Stimulus)
{
	// 여기서 외적인 연출이나 그런거 해도 될듯?

}
void UYSAIPerceptionComponent::OnActorForgotten(AActor* Actor)
{
}

