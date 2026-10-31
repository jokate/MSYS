// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "YSCombatHUDWidget.generated.h"

class UYSTargetMarkerWidget;
class UYSHUDWindowRowWidget;
class UYSGroggyGaugeWidget;
class UYSSwitchAttackToastWidget;
class UYSNearbyRadarWidget;

/** 전투 HUD 루트. 요소 위젯을 묶기만 한다. 무엇을 언제 보일지는 UYSCombatHUDComponent 가 이벤트를 받아 각 요소에 내린다. */
UCLASS(Abstract)
class MNYS_API UYSCombatHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public :
	UYSTargetMarkerWidget* GetTargetMarker() const { return TargetMarker; }
	UYSHUDWindowRowWidget* GetWindowRow() const { return WindowRow; }
	UYSGroggyGaugeWidget* GetGroggyGauge() const { return GroggyGauge; }
	UYSSwitchAttackToastWidget* GetSwitchAttackToast() const { return SwitchAttackToast; }
	UYSNearbyRadarWidget* GetNearbyRadar() const { return NearbyRadar; }

protected :
	UPROPERTY(BlueprintReadOnly, Category = "YS | HUD", meta = (BindWidgetOptional))
	TObjectPtr<UYSTargetMarkerWidget> TargetMarker;

	UPROPERTY(BlueprintReadOnly, Category = "YS | HUD", meta = (BindWidgetOptional))
	TObjectPtr<UYSHUDWindowRowWidget> WindowRow;

	UPROPERTY(BlueprintReadOnly, Category = "YS | HUD", meta = (BindWidgetOptional))
	TObjectPtr<UYSGroggyGaugeWidget> GroggyGauge;

	UPROPERTY(BlueprintReadOnly, Category = "YS | HUD", meta = (BindWidgetOptional))
	TObjectPtr<UYSSwitchAttackToastWidget> SwitchAttackToast;

	UPROPERTY(BlueprintReadOnly, Category = "YS | HUD", meta = (BindWidgetOptional))
	TObjectPtr<UYSNearbyRadarWidget> NearbyRadar;
};
