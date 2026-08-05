// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "YSPlayerController.generated.h"

class UYSTargetingComponent;
class UYSCameraManageComponent;
class UYSTargetingComponent;
class UInputMappingContext;
/**
 * 
 */
UCLASS()
class MNYS_API AYSPlayerController : public APlayerController
{
	GENERATED_BODY()

public :
	AYSPlayerController(const FObjectInitializer& ObjectInitializer);
	virtual void SetupInputComponent() override;
	
protected : 
	virtual void AddYawInput(float Val) override;
	
public :
	UPROPERTY(EditDefaultsOnly, Category="Input|Input Mappings")
	TObjectPtr<UInputMappingContext> UIInputMappingContext;
	
	UPROPERTY(EditDefaultsOnly, Category="YS | Lock On")
	TObjectPtr<UYSCameraManageComponent> LockOnComponent;

	// 폰이 아니라 컨트롤러에 둔다. 스쿼드 태그 교대로 폰이 바뀌어도
	// 인디케이터 인스턴스와 조준 상태가 유지돼야 하기 때문이다.
	UPROPERTY(EditDefaultsOnly, Category="YS | Comp")
	TObjectPtr<UYSTargetingComponent> TargetingComponent;

protected : 
	UPROPERTY(EditDefaultsOnly)
	float LockOnBreakThreshold = 3.0f; 
};
