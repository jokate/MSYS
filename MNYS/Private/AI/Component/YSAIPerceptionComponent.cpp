// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Component/YSAIPerceptionComponent.h"

#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Sight.h"


// Sets default values for this component's properties
UYSAIPerceptionComponent::UYSAIPerceptionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UYSAIPerceptionComponent::BeginPlay()
{
	Super::BeginPlay();

	OnTargetPerceptionUpdated.AddDynamic(this, &UYSAIPerceptionComponent::OnPerceptionUpdated_Callback);
	OnTargetPerceptionInfoUpdated.AddDynamic(this, &UYSAIPerceptionComponent::OnTargetPerceptionInfoUpdated_Callback);
	OnTargetPerceptionForgotten.AddDynamic(this, &UYSAIPerceptionComponent::OnActorForgotten);
}


// Called every frame
void UYSAIPerceptionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                             FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UYSAIPerceptionComponent::OnPerceptionUpdated_Callback(AActor* Actor, FAIStimulus Stimulus)
{
	// 본 경우..
	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
	{
		
	}
	
	if ( Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
	{
		
	}
}

