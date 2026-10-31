// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "YSGroggyGaugeWidget.generated.h"

class UProgressBar;

UENUM(BlueprintType)
enum class EYSGroggyGaugeState : uint8
{
	Hidden		UMETA(DisplayName = "숨김"),
	Gauge		UMETA(DisplayName = "게이지"),
	Broken		UMETA(DisplayName = "브레이크"),
};

/** 그로기 게이지와 브레이크 표시. 상단 중앙 고정. 상태와 값은 UYSCombatHUDComponent 가 내리고, 브레이크 중에는 게이지 자리에 BrokenPanel 을 띄운다. */
UCLASS(Abstract)
class MNYS_API UYSGroggyGaugeWidget : public UUserWidget
{
	GENERATED_BODY()

public :
	void SetGaugeState(EYSGroggyGaugeState NewState);
	void SetGroggy(float Groggy, float MaxGroggy);

	UFUNCTION(BlueprintPure, Category = "YS | HUD")
	EYSGroggyGaugeState GetGaugeState() const { return GaugeState; }

protected :
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "YS | HUD")
	void OnGaugeStateChanged(EYSGroggyGaugeState NewState);

private :
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> GroggyBar;

	// 「얼차려」 등 브레이크 표지 묶음
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> BrokenPanel;

	EYSGroggyGaugeState GaugeState = EYSGroggyGaugeState::Hidden;
};
