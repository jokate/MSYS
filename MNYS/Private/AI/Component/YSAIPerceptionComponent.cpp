// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Component/YSAIPerceptionComponent.h"

#include "AI/YSAIController.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Sight.h"


UYSAIPerceptionComponent::UYSAIPerceptionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

TSharedPtr<FYSTargetingActorCollections> UYSAIPerceptionComponent::GetTargetingCollection(const AActor* Actor)
{
	if (IsValid(Actor) == false )
	{
		return nullptr;
	}

	const AYSAIController* AIController = Cast<AYSAIController>(Actor);
	if ( AIController == nullptr )
	{
		AIController = Actor->GetInstigatorController<AYSAIController>();
	}

	if (IsValid(AIController) == false || IsValid(AIController->MainPerceptionComponent) == false )
	{
		return nullptr;
	}

	return AIController->MainPerceptionComponent->GetTargetingActorCollections();
}

AActor* UYSAIPerceptionComponent::GetBestTarget(const AActor* Actor)
{
	const TSharedPtr<FYSTargetingActorCollections> TargetingCollections = GetTargetingCollection(Actor);
	return TargetingCollections.IsValid() ? TargetingCollections->GetBestTargetActor() : nullptr;
}

bool UYSAIPerceptionComponent::MatchesAffiliation(const FActorPerceptionInfo& Info, EYSPerceptionAffiliation Affiliation)
{
	switch (Affiliation)
	{
		case EYSPerceptionAffiliation::Hostile:  return Info.bIsHostile;
		case EYSPerceptionAffiliation::Friendly: return Info.bIsFriendly;
		case EYSPerceptionAffiliation::Neutral:  return Info.bIsHostile == false && Info.bIsFriendly == false;
		default: return true;
	}
}


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


void UYSAIPerceptionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                             FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}


void UYSAIPerceptionComponent::OnPerceptionUpdated_Callback(AActor* Actor, FAIStimulus Stimulus)
{

}
void UYSAIPerceptionComponent::OnActorForgotten(AActor* Actor)
{
}

