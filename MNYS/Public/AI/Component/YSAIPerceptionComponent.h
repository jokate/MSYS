// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "General/YSEnum.h"
#include "General/YSStruct.h"
#include "Perception/AIPerceptionComponent.h"
#include "YSAIPerceptionComponent.generated.h"


// 퍼셉션마다 의도하는 위치 · 방식이 달라, 감지된 액터를 여기서 한 목록으로 묶어 찾게 한다.

struct FYSTargetingActorCollections;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MNYS_API UYSAIPerceptionComponent : public UAIPerceptionComponent
{
	GENERATED_BODY()

public:
	UYSAIPerceptionComponent();

	// AI 의 폰 · 컨트롤러 어느 쪽을 받아도 된다. 퍼셉션이나 목록이 없으면 nullptr.
	static TSharedPtr<FYSTargetingActorCollections> GetTargetingCollection(const AActor* Actor);
	static AActor* GetBestTarget(const AActor* Actor);
	static bool MatchesAffiliation(const FActorPerceptionInfo& Info, EYSPerceptionAffiliation Affiliation);

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;


protected:
	UFUNCTION()
	void OnPerceptionUpdated_Callback(AActor* Actor, FAIStimulus Stimulus);
	
	UFUNCTION()
	void OnActorForgotten(AActor* Actor);
	
public : 
	
	const TSharedPtr<FYSTargetingActorCollections>& GetTargetingActorCollections() const
	{
		return TargetingActorCollections;
	}
	
	void SetTargetActorCollections(const TSharedPtr<FYSTargetingActorCollections>& InTargetingActorCollections)
	{
		TargetingActorCollections = InTargetingActorCollections;
	}

private :
	TSharedPtr<FYSTargetingActorCollections> TargetingActorCollections;
};
