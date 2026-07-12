// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

/**
 * 
 */
class SResponseCurvePreviewWidget : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SResponseCurvePreviewWidget) {}

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual FVector2D ComputeDesiredSize(float) const override;
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
						  int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	TWeakObjectPtr<class UYSAIAbilityScoreFunctionBase> ResponseCurve;

private:
	FVector2D GetWidgetPosition(float X, float Y, const FGeometry& Geom) const;
};
