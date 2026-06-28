// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/EQS/Test/YSEnvQueryTest_TeamCheck.h"

#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionListenerInterface.h"
#include "EnvironmentQuery/EnvQueryTypes.h"

#define LOCTEXT_NAMESPACE "YSEnvQueryTest_TeamCheck"

UYSEnvQueryTest_TeamCheck::UYSEnvQueryTest_TeamCheck()
{
	TestPurpose = EEnvTestPurpose::Type::Filter;
	FilterType = EEnvTestFilterType::Match;
}

void UYSEnvQueryTest_TeamCheck::RunTest(FEnvQueryInstance& QueryInstance) const
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
	QueryInstance.PrepareContext(ContextCheck, Listeners);

	for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
	{
		AActor* ItemActor = GetItemActor(QueryInstance, It.GetIndex());
		
		if ( IsValid(ItemActor) == false )
		{
			continue;
		}
		
		
		bool bFinalRes = false;
		for (AActor* ListenerActor : Listeners)
		{
			IAIPerceptionListenerInterface* Listener = Cast<IAIPerceptionListenerInterface>(ListenerActor);
		
			if ( Listener == nullptr || IsValid(Listener->GetPerceptionComponent()) == false )
			{
				continue;
			}
		
			UAIPerceptionComponent* PerceptionComponent = Listener->GetPerceptionComponent();
		
			const FActorPerceptionInfo* Info = PerceptionComponent->GetActorInfo(*ItemActor);
			if (Info == nullptr )
			{
				continue;
			}
			
			switch (Affiliation)
			{
				case EYSPerceptionAffiliation::Hostile:  if (Info->bIsHostile == false)  { continue; } break;
				case EYSPerceptionAffiliation::Friendly: if (Info->bIsFriendly == false)  { continue; } break;
				case EYSPerceptionAffiliation::Neutral:  if (Info->bIsHostile || Info->bIsFriendly) { continue; } break;
			default: break; 
			}
			
			bFinalRes = true; 
		}
		
		It.SetScore(TestPurpose, FilterType, bFinalRes, true);
	}
}

FText UYSEnvQueryTest_TeamCheck::GetDescriptionTitle() const
{
	// 베이스 제목(필터/스코어 표기) 뒤에 제휴 종류를 덧붙인다.
	return FText::Format(
		LOCTEXT("TeamCheckTitle", "{0}: 제휴 {1}"),
		Super::GetDescriptionTitle(),
		UEnum::GetDisplayValueAsText(Affiliation));
}

FText UYSEnvQueryTest_TeamCheck::GetDescriptionDetails() const
{
	FFormatNamedArguments Args;
	Args.Add(TEXT("Context"),     UEnvQueryTypes::DescribeContext(ContextCheck));
	Args.Add(TEXT("Affiliation"), UEnum::GetDisplayValueAsText(Affiliation));

	return FText::Format(
		LOCTEXT("TeamCheckDetails", "퍼셉션 소스: {Context}\n제휴 필터: {Affiliation}"),
		Args);
}

#undef LOCTEXT_NAMESPACE
