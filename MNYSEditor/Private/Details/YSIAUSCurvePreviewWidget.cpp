// Fill out your copyright notice in the Description page of Project Settings.


#include "Details/YSIAUSCurvePreviewWidget.h"

#include "Ability/AIAbility/YSAIAbilityScoreFunction.h"

void SResponseCurvePreviewWidget::Construct(const FArguments& InArgs)
{
}

FVector2D SResponseCurvePreviewWidget::ComputeDesiredSize(float) const
{
	return FVector2D(128, 92);
}

int32 SResponseCurvePreviewWidget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
	const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	bool bEnabled = ShouldBeEnabled(bParentEnabled);
	ESlateDrawEffect DrawEffects = bEnabled ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;
	const FSlateBrush* TimelineAreaBrush = FAppStyle::GetBrush("Profiler.LineGraphArea");
	const FSlateBrush* WhiteBrush = FAppStyle::GetBrush("WhiteTexture");

	// Draw timeline background
	FSlateDrawElement::MakeBox(OutDrawElements, LayerId,
							   AllottedGeometry.ToPaintGeometry(FVector2D(0, 0), FSlateLayoutTransform(FVector2D(AllottedGeometry.Size.X, AllottedGeometry.Size.Y))),
							   TimelineAreaBrush, DrawEffects, TimelineAreaBrush->GetTint(InWidgetStyle) * InWidgetStyle.GetColorAndOpacityTint());

	LayerId++;

	if (ResponseCurve == nullptr)
	{
		return LayerId;
	}

	// Draw axies
	TArray<FVector2D> AxisPoints;
	AxisPoints.Add(GetWidgetPosition(0, 1, AllottedGeometry));
	AxisPoints.Add(GetWidgetPosition(0, 0, AllottedGeometry));
	AxisPoints.Add(GetWidgetPosition(1, 0, AllottedGeometry));

	FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), AxisPoints, DrawEffects,
								 WhiteBrush->GetTint(InWidgetStyle) * InWidgetStyle.GetColorAndOpacityTint());

	LayerId++;

	// Draw line graph
	TArray<FVector2D> LinePoints;

	for (int i = 0; i <= 1000; i++)
	{
		float XVal = i / 1000.0f;
		float YVal = ResponseCurve->ProcessIAUSFunction(XVal);

		const float XPos = XVal * AllottedGeometry.Size.X;
		const float YPos = (1.0 - YVal) * AllottedGeometry.Size.Y;

		LinePoints.Add(FVector2D(FMath::TruncToInt(XPos), FMath::TruncToInt(YPos)));
	}

	FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), LinePoints, DrawEffects,
								 InWidgetStyle.GetColorAndOpacityTint() * FLinearColor(0.0f, 0.0f, 1.0f, 1.0f));

	LayerId++;

	return LayerId;
}

FVector2D SResponseCurvePreviewWidget::GetWidgetPosition(float X, float Y, const FGeometry& Geom) const
{
	return FVector2D((X * Geom.GetLocalSize().X), (Geom.GetLocalSize().Y - 1) - (Y * Geom.GetLocalSize().Y));
}
