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
	// Sets default values for this component's properties
	UYSPhaseChangeComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
private : 
	void CheckPhaseChange();

public :
	// 나중에 AND OR 붙을 수 있음 이거 변경 필요함.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | PhaseChange")
	TArray<FYSPhaseChangeConditions> PhaseChangeConditions;
	
private : 
	// 페이즈는 1번부터 시작한다.
	int32 CurrentPhase = 1;
	TWeakObjectPtr<AYSCharacterBase> OwnerCharacter;
};
