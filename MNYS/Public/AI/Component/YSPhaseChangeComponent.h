// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "YSPhaseChangeCondition.h"
#include "Components/ActorComponent.h"
#include "StructUtils/InstancedStruct.h"
#include "YSPhaseChangeComponent.generated.h"


class AYSCharacterBase;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MNYS_API UYSPhaseChangeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UYSPhaseChangeComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
private : 
	void CheckPhaseChange();
	bool IsDeferred(AActor* Owner) const;

public :
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | PhaseChange")
	TArray<FYSPhaseChangeConditions> PhaseChangeConditions;

	// 이 태그 중 하나라도 있으면 조건이 맞아도 전환을 미룬다 — 태그가 빠진 뒤 전환한다 (기본: 브레이크 중).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | PhaseChange", meta = (DisplayName = "전환을 미루는 태그"))
	TArray<FGameplayTag> DeferTags;

private : 
	// 페이즈는 1번부터 시작한다.
	int32 CurrentPhase = 1;
	TWeakObjectPtr<AYSCharacterBase> OwnerCharacter;
};
