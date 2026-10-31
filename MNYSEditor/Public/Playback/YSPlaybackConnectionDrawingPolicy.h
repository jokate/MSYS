// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ConnectionDrawingPolicy.h"
#include "EdGraphUtilities.h"

class UEdGraph;

// 데이터상 연결은 [상태A]→[전환]→[상태B] 두 개지만, DetermineLinkGeometry 에서 양 끝 기하를
// 상태A·상태B 로 바꿔치기해 선 하나로 그린다. 그래서 전환 노드가 선 위에 얹힌 아이콘처럼 보인다.
class FYSPlaybackConnectionDrawingPolicy : public FConnectionDrawingPolicy
{
public:
	FYSPlaybackConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float ZoomFactor,
	                                   const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements, UEdGraph* InGraphObj);

	//~ FConnectionDrawingPolicy
	virtual void DetermineWiringStyle(UEdGraphPin* OutputPin, UEdGraphPin* InputPin, FConnectionParams& Params) override;
	virtual void DetermineLinkGeometry(FArrangedChildren& ArrangedNodes, TSharedRef<SWidget>& OutputPinWidget,
	                                   UEdGraphPin* OutputPin, UEdGraphPin* InputPin,
	                                   FArrangedWidget*& StartWidgetGeometry, FArrangedWidget*& EndWidgetGeometry) override;
	virtual void Draw(TMap<TSharedRef<SWidget>, FArrangedWidget>& PinGeometries, FArrangedChildren& ArrangedNodes) override;
	virtual void DrawSplineWithArrow(const FGeometry& StartGeom, const FGeometry& EndGeom, const FConnectionParams& Params) override;
	virtual void DrawSplineWithArrow(const FVector2f& StartPoint, const FVector2f& EndPoint, const FConnectionParams& Params) override;
	virtual void DrawPreviewConnector(const FGeometry& PinGeometry, const FVector2f& StartPoint, const FVector2f& EndPoint, UEdGraphPin* Pin) override;
	virtual FVector2f ComputeSplineTangent(const FVector2f& Start, const FVector2f& End) const override;
	//~ End

protected:
	void DrawLineWithArrow(const FVector2f& StartAnchorPoint, const FVector2f& EndAnchorPoint, const FConnectionParams& Params);

	UEdGraph* GraphObj = nullptr;

	TMap<UEdGraphNode*, int32> NodeWidgetMap;
};


class FYSPlaybackConnectionFactory : public FGraphPanelPinConnectionFactory
{
public:
	virtual class FConnectionDrawingPolicy* CreateConnectionPolicy(const class UEdGraphSchema* Schema, int32 InBackLayerID, int32 InFrontLayerID,
	                                                               float ZoomFactor, const FSlateRect& InClippingRect,
	                                                               FSlateWindowElementList& InDrawElements, UEdGraph* InGraphObj) const override;
};
