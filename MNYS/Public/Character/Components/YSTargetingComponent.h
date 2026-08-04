// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "General/YSStruct.h"
#include "StructUtils/InstancedStruct.h"
#include "Targeting/YSSkillIndicator.h"
#include "Targeting/YSTargetingShape.h"
#include "YSTargetingComponent.generated.h"

class AYSSkillIndicator;

/** 스킬 하나가 "어떻게 조준되는가"를 선언합니다. 어빌리티가 EditDefaultsOnly로 들고 있습니다. */
USTRUCT(BlueprintType)
struct FYSTargetingSpec
{
	GENERATED_BODY()

	// 치수(사거리·반경·각도)와 사거리 링 표시 여부는 전부 도형이 들고 간다.
	// 도형을 고르면 그 도형의 필드만 뜨므로 EditCondition 을 늘어놓을 필요가 없다.
	UPROPERTY(EditDefaultsOnly, Category = "YS | Targeting",
		meta = (DisplayName = "타게팅 도형",
				BaseStruct = "/Script/MNYS.YSTargetingShape", ExcludeBaseStruct))
	FInstancedStruct Shape;

	UPROPERTY(EditDefaultsOnly, Category = "YS | Targeting", meta = (DisplayName = "조준 중 카메라"))
	FYSCameraEffectParams CameraParams;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MNYS_API UYSTargetingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UYSTargetingComponent();

	static UYSTargetingComponent* Get(const AActor* Actor);
	
	bool BeginTargeting(UObject* Requester, const FYSTargetingSpec& Spec);
	
	void EndTargeting(UObject* Requester);

	UFUNCTION(BlueprintPure, Category = "YS | Targeting")
	bool IsTargeting() const { return CurrentRequester.IsValid(); }
	
	const FYSTargetingResult& GetResult() const { return CurrentResult; }

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	FYSTargetingResult EvaluateTarget() const;

	/** 현재 도형·결과를 인디케이터에 반영한다. 매 프레임 호출을 전제로 한다. */
	void RefreshIndicator();

	/** 화면 기준점을 월드 지면으로 옮긴다. 허공을 겨눈 경우 수평면으로 폴백한다. */
	bool ProjectScreenToGround(FVector& OutLocation) const;

	/** XY만 바뀐 지점의 Z를 다시 지면에 붙인다. 안 하면 경사에서 공중에 뜬다. */
	FVector SnapToGround(const FVector& Location) const;

	FVector GetCasterLocation() const;
	FVector GetCasterForward() const;

	void EndTargetingInternal();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "YS | Targeting", meta = (DisplayName = "인디케이터 클래스"))
	TSubclassOf<AYSSkillIndicator> IndicatorClass;

	UPROPERTY(EditDefaultsOnly, Category = "YS | Targeting", meta = (DisplayName = "조준 트레이스 최대 거리"))
	float MaxTraceDistance = 20000.f;

	/**
	 * 조준 트레이스 채널.
	 * 기본값 Visibility 는 적과 소품에도 맞아 적 뒤를 겨누면 조준점이 몸통에 붙는다.
	 * 지형만 잡는 전용 채널을 파서 지정하는 것을 권장한다.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "YS | Targeting", meta = (DisplayName = "조준 트레이스 채널"))
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

private:
	// 최초 1회만 스폰하고 계속 재사용한다. 레디할 때마다 스폰/디스트로이 하지 않는다.
	UPROPERTY()
	TObjectPtr<AYSSkillIndicator> Indicator;

	// 약참조인 이유 — 어빌리티가 EndAbility를 못 거치고 파괴돼도 조준이 자동 종료된다.
	UPROPERTY()
	TWeakObjectPtr<UObject> CurrentRequester;

	UPROPERTY()
	FYSTargetingSpec CurrentSpec;

	FYSTargetingResult CurrentResult;
};
