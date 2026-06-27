// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Perception/AIPerceptionComponent.h"
#include "YSAIPerceptionComponent.generated.h"


/*
 * 퍼셉션에 감지된 액터를 간단하게 찾을 수 있도록 해야 한다.
 * 사유는 각 퍼셉션 별로 의도하는 위치 및 의도하는 방식이 존재하기 때문임 
 * 잡몹의 경우에는 사용 가능하되, 보스의 경우 ( 필드에 깔린 경우 예외 ) 강제적으로 선제 타겟팅을 걸도록 해야 한다.
 * 사실상 솔로플레이기 때문에, 만약에 소환수를 까는 경우 대비
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MNYS_API UYSAIPerceptionComponent : public UAIPerceptionComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UYSAIPerceptionComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
	
};
