// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/YSGroggyGaugeWidget.h"

#include "Components/ProgressBar.h"

void UYSGroggyGaugeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	GaugeState = EYSGroggyGaugeState::Gauge;
	SetGaugeState(EYSGroggyGaugeState::Hidden);
}

void UYSGroggyGaugeWidget::SetGaugeState(EYSGroggyGaugeState NewState)
{
	if ( GaugeState == NewState )
	{
		return;
	}

	GaugeState = NewState;

	SetVisibility(GaugeState == EYSGroggyGaugeState::Hidden ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	GroggyBar->SetVisibility(GaugeState == EYSGroggyGaugeState::Gauge ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);

	if ( IsValid(BrokenPanel) )
	{
		BrokenPanel->SetVisibility(GaugeState == EYSGroggyGaugeState::Broken ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}

	OnGaugeStateChanged(GaugeState);
}

void UYSGroggyGaugeWidget::SetGroggy(float Groggy, float MaxGroggy)
{
	GroggyBar->SetPercent(MaxGroggy > 0.f ? FMath::Clamp(Groggy / MaxGroggy, 0.f, 1.f) : 0.f);
}
