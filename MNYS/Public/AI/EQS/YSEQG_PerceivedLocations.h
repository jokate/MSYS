// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "YSEnvQueryGeneratorBase.h"
#include "General/YSEnum.h"
#include "YSEQG_PerceivedLocations.generated.h"

/**
 * 퍼셉션으로 감지된 '자극 위치'를 Point 아이템으로 생성한다.
 * 시각의 실시간 위치가 아니라 자극 발생 위치(GetStimulusLocation)를 쓰므로 청각/놓침 조사에 적합.
 */
UCLASS()
class MNYS_API UYSEQG_PerceivedLocations : public UYSEnvQueryGeneratorBase
{
	GENERATED_BODY()


public :
	UYSEQG_PerceivedLocations();
	virtual void GenerateItems(FEnvQueryInstance& QueryInstance) const override;

protected :
	UPROPERTY(EditAnywhere, Category="YS | Generator", meta = (DisplayName = "퍼셉션 소스 컨텍스트"))
	TSubclassOf<UEnvQueryContext> ListenerContext;

	UPROPERTY(EditAnywhere, Category="YS | Generator", meta = (DisplayName = "놓친(기억) 대상 포함 여부"))
	bool bIncludeKnownActors = true;

	UPROPERTY(EditAnywhere, Category="YS | Generator", meta = (DisplayName = "사용할 감각 종류"))
	TSubclassOf<class UAISense> SenseToUse;

	UPROPERTY(EditAnywhere, Category="YS | Generator", meta = (DisplayName = "허용 액터 클래스"))
	TSubclassOf<AActor> AllowedActorClass;

	UPROPERTY(EditAnywhere, Category="YS | Generator", meta = (DisplayName = "제휴 필터"))
	EYSPerceptionAffiliation AffiliationFilter = EYSPerceptionAffiliation::Hostile;
};
