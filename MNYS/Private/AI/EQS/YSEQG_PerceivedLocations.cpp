// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/EQS/YSEQG_PerceivedLocations.h"

#include "EnvironmentQuery/Items/EnvQueryItemType_Point.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionListenerInterface.h"
#include "Perception/AISense.h"
#include "Perception/AISense_Hearing.h"
#include "GameFramework/Pawn.h"
#include "Engine/Engine.h"
#include "VisualLogger/VisualLogger.h"

UYSEQG_PerceivedLocations::UYSEQG_PerceivedLocations()
{
	ItemType = UEnvQueryItemType_Point::StaticClass();
	
	SenseToUse = UAISense_Hearing::StaticClass();
}

void UYSEQG_PerceivedLocations::GenerateItems(FEnvQueryInstance& QueryInstance) const
{
	UObject* QueryOwner = QueryInstance.Owner.Get();
	if (QueryOwner == nullptr)
	{	
		return;
	}
		
	UWorld* World = GEngine->GetWorldFromContextObject(QueryOwner, EGetWorldErrorMode::LogAndReturnNull);
	if (World == nullptr)
	{
		return;
	}

	TArray<AActor*> AllPerceivedActors;
	TArray<AActor*> Listeners;
	QueryInstance.PrepareContext(ListenerContext, Listeners);

	for (AActor* ListenerActor : Listeners)
	{
		IAIPerceptionListenerInterface* Listener = Cast<IAIPerceptionListenerInterface>(ListenerActor);
		if (Listener == nullptr && ListenerActor != nullptr)
		{
			APawn* ListenerAsPawn = Cast<APawn>(QueryOwner);
			if (IsValid(ListenerAsPawn))
			{
				Listener = Cast<IAIPerceptionListenerInterface>(ListenerAsPawn->GetController());
			}
		}

		if (Listener == nullptr)
		{
			UE_VLOG(QueryOwner, LogEQS, Error, TEXT("Tried to use UYSEQG_PerceivedLocations while query context actor %s doesn\'t represent a valid perception listener")
				, *GetNameSafe(ListenerActor));
			continue;
		}

		UAIPerceptionComponent* PerceptionComponent = Listener->GetPerceptionComponent();
		if (IsValid(PerceptionComponent) == false)
		{
			UE_VLOG(QueryOwner, LogEQS, Error, TEXT("Tried to use UYSEQG_PerceivedLocations while query context actor\'s %s UAIPerceptionComponent is missing")
				, *ListenerActor->GetName());
			continue;
		}

		TArray<AActor*> LocalPerceivedActors;
		if (bIncludeKnownActors)
		{
			PerceptionComponent->GetKnownPerceivedActors(SenseToUse, LocalPerceivedActors);
		}
		else
		{
			PerceptionComponent->GetCurrentlyPerceivedActors(SenseToUse, LocalPerceivedActors);
		}

		// SenseToUse 가 지정되면 해당 감각의 자극 위치를, 미지정이면 가장 최근 자극 위치를 사용한다.
		const FAISenseID SenseID = UAISense::GetSenseID(SenseToUse);

		for (AActor* PerceivedActor : LocalPerceivedActors)
		{
			if (IsValid(PerceivedActor) == false)
			{
				continue;
			}

			const FActorPerceptionInfo* Info = PerceptionComponent->GetActorInfo(*PerceivedActor);
			if (Info == nullptr )
			{
				continue;
			}
			
			switch (AffiliationFilter)
			{
				case EYSPerceptionAffiliation::Hostile:  if (Info->bIsHostile == false)  { continue; } break;
				case EYSPerceptionAffiliation::Friendly: if (Info->bIsFriendly == false)  { continue; } break;
				case EYSPerceptionAffiliation::Neutral:  if (Info->bIsHostile || Info->bIsFriendly) { continue; } break;
				default: break; 
			}
			
			if (AllowedActorClass && PerceivedActor->IsA(AllowedActorClass.Get()) == false)
			{
				continue;
			}

			const FVector StimulusLocation = SenseID.IsValid() ? Info->GetStimulusLocation(SenseID)
			                                                   : Info->GetLastStimulusLocation();
			if (FAISystem::IsValidLocation(StimulusLocation))
			{
				QueryInstance.AddItemData<UEnvQueryItemType_Point>(StimulusLocation);
			}
		}
	}
}
