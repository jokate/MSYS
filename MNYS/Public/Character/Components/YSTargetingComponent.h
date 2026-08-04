// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "General/YSStruct.h"
#include "YSTargetingComponent.generated.h"

class AYSSkillIndicator;
/** 스킬 하나가 "어떻게 조준되는가"를 선언합니다. 어빌리티가 EditDefaultsOnly로 들고 있습니다. */
USTRUCT(BlueprintType)
struct FYSTargetingSpec
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "YS | Targeting", meta = (DisplayName = "최대 사거리"))
	float Range = 1000.f;

	UPROPERTY(EditDefaultsOnly, Category = "YS | Targeting", meta = (DisplayName = "반경"))
	float Radius = 200.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "YS | Targeting", meta = (DisplayName = "사거리 밖이면 끝으로 클램프"))
	bool bClampToRange = true;
	
	UPROPERTY(EditDefaultsOnly, Category = "YS | Targeting", meta = (DisplayName = "조준 중 카메라"))
	FYSCameraEffectParams CameraParams;
};

// 결과를 어떻게 보일 거 ㅅ인가 
USTRUCT(BlueprintType)
struct FYSTargetingResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "YS | Targeting")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "YS | Targeting")
	FVector Direction = FVector::ForwardVector;
	
	UPROPERTY(BlueprintReadOnly, Category = "YS | Targeting")
	AActor* TargetingActor = nullptr;

	/** 사거리 밖·조준 실패 등으로 시전할 수 없으면 false. */
	UPROPERTY(BlueprintReadOnly, Category = "YS | Targeting")
	bool bValid = false;
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

	FYSTargetingSpec   CurrentSpec;
	FYSTargetingResult CurrentResult;
};
