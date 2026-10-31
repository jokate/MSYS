// Fill out your copyright notice in the Description page of Project Settings.


#include "Playback/YSPlaybackConnectionDrawingPolicy.h"

#include "EdGraph/EdGraphPin.h"
#include "Layout/ArrangedChildren.h"
#include "Layout/ArrangedWidget.h"
#include "Playback/YSPlaybackGraphNode.h"
#include "Playback/YSPlaybackGraphSchema.h"
#include "SGraphNode.h"
#include "Styling/AppStyle.h"

FYSPlaybackConnectionDrawingPolicy::FYSPlaybackConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float ZoomFactor,
                                                                       const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements, UEdGraph* InGraphObj)
	: FConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, ZoomFactor, InClippingRect, InDrawElements)
	, GraphObj(InGraphObj)
{
	ArrowImage = FAppStyle::GetBrush(TEXT("Graph.AnimStateNode.ConnectionArrow"));
}

void FYSPlaybackConnectionDrawingPolicy::DetermineWiringStyle(UEdGraphPin* OutputPin, UEdGraphPin* InputPin, FConnectionParams& Params)
{
	Params.AssociatedPin1 = OutputPin;
	Params.AssociatedPin2 = InputPin;
	Params.WireThickness = 1.5f;
	Params.WireColor = FLinearColor(0.72f, 0.72f, 0.72f);

	// 전환 아이콘 색과 맞춘다.
	if (InputPin != nullptr)
	{
		if (const UYSPlaybackGraphNode_Transition* TransitionNode = Cast<UYSPlaybackGraphNode_Transition>(InputPin->GetOwningNode()))
		{
			if (TransitionNode->IsDangling() || TransitionNode->HasValidOutput() == false)
			{
				Params.WireColor = FLinearColor(0.90f, 0.20f, 0.15f);
			}
			else if (TransitionNode->TransitionConditions.Num() > 0)
			{
				Params.WireColor = FLinearColor(0.95f, 0.70f, 0.25f);
			}
		}
	}

	if (HoveredPins.Num() > 0)
	{
		ApplyHoverDeemphasis(OutputPin, InputPin, Params.WireThickness, Params.WireColor);
	}
}

void FYSPlaybackConnectionDrawingPolicy::DetermineLinkGeometry(FArrangedChildren& ArrangedNodes, TSharedRef<SWidget>& OutputPinWidget,
                                                               UEdGraphPin* OutputPin, UEdGraphPin* InputPin,
                                                               FArrangedWidget*& StartWidgetGeometry, FArrangedWidget*& EndWidgetGeometry)
{
	if (OutputPin->GetOwningNode()->IsA<UYSPlaybackGraphNode_Entry>())
	{
		StartWidgetGeometry = PinGeometries->Find(OutputPinWidget);

		if (const int32* StateIndex = NodeWidgetMap.Find(InputPin->GetOwningNode()))
		{
			EndWidgetGeometry = &(ArrangedNodes[*StateIndex]);
		}

		return;
	}

	// 상태 → 전환 연결의 양 끝을 이전 상태와 다음 상태로 바꿔치기해야 두 연결이 선 하나로 합쳐진다.
	if (const UYSPlaybackGraphNode_Transition* TransitionNode = Cast<UYSPlaybackGraphNode_Transition>(InputPin->GetOwningNode()))
	{
		UYSPlaybackGraphNode_State* PrevState = TransitionNode->GetPreviousState();
		UYSPlaybackGraphNode_Base* NextNode = TransitionNode->GetTargetNode();

		if (PrevState == nullptr || NextNode == nullptr)
		{
			return;
		}

		const int32* PrevIndex = NodeWidgetMap.Find(PrevState);
		const int32* NextIndex = NodeWidgetMap.Find(NextNode);

		if (PrevIndex != nullptr && NextIndex != nullptr)
		{
			StartWidgetGeometry = &(ArrangedNodes[*PrevIndex]);
			EndWidgetGeometry = &(ArrangedNodes[*NextIndex]);
		}
	}

	// 전환 → 상태 연결은 기하를 비워 둔다 — 엔진이 건너뛰어 선이 두 번 그려지는 걸 막는다.
}

void FYSPlaybackConnectionDrawingPolicy::Draw(TMap<TSharedRef<SWidget>, FArrangedWidget>& InPinGeometries, FArrangedChildren& ArrangedNodes)
{
	NodeWidgetMap.Empty(ArrangedNodes.Num());

	for (int32 NodeIndex = 0; NodeIndex < ArrangedNodes.Num(); ++NodeIndex)
	{
		const FArrangedWidget& CurWidget = ArrangedNodes[NodeIndex];
		const TSharedRef<SGraphNode> ChildNode = StaticCastSharedRef<SGraphNode>(CurWidget.Widget);

		NodeWidgetMap.Add(ChildNode->GetNodeObj(), NodeIndex);
	}

	FConnectionDrawingPolicy::Draw(InPinGeometries, ArrangedNodes);
}

void FYSPlaybackConnectionDrawingPolicy::DrawSplineWithArrow(const FGeometry& StartGeom, const FGeometry& EndGeom, const FConnectionParams& Params)
{
	// 중심끼리 이으면 선이 상자를 파고들므로 최단 지점끼리 잇는다.
	const FVector2f StartCenter = FGeometryHelper::CenterOf(StartGeom);
	const FVector2f EndCenter = FGeometryHelper::CenterOf(EndGeom);
	const FVector2f SeedPoint = (StartCenter + EndCenter) * 0.5f;

	const FVector2f StartAnchorPoint = FGeometryHelper::FindClosestPointOnGeom(StartGeom, SeedPoint);
	const FVector2f EndAnchorPoint = FGeometryHelper::FindClosestPointOnGeom(EndGeom, SeedPoint);

	DrawSplineWithArrow(StartAnchorPoint, EndAnchorPoint, Params);
}

void FYSPlaybackConnectionDrawingPolicy::DrawSplineWithArrow(const FVector2f& StartPoint, const FVector2f& EndPoint, const FConnectionParams& Params)
{
	DrawLineWithArrow(StartPoint, EndPoint, Params);
}

void FYSPlaybackConnectionDrawingPolicy::DrawLineWithArrow(const FVector2f& StartAnchorPoint, const FVector2f& EndAnchorPoint, const FConnectionParams& Params)
{
	// A→B 와 B→A 가 겹치지 않도록 수직으로 6픽셀 민다.
	const float LineSeparationAmount = 6.f * ZoomFactor;

	const FVector2f DeltaPos = EndAnchorPoint - StartAnchorPoint;
	const FVector2f UnitDelta = DeltaPos.GetSafeNormal();
	const FVector2f Normal = FVector2f(DeltaPos.Y, -DeltaPos.X).GetSafeNormal();

	const FVector2f DirectionBias = Normal * LineSeparationAmount;
	const FVector2f LengthBias = ArrowRadius.X * UnitDelta;

	const FVector2f StartPoint = StartAnchorPoint + DirectionBias + LengthBias;
	const FVector2f EndPoint = EndAnchorPoint + DirectionBias - LengthBias;

	DrawConnection(WireLayerID, StartPoint, EndPoint - (LengthBias * 0.8f), Params);

	const FVector2f ArrowDrawPos = EndPoint - ArrowRadius;
	const double AngleInRadians = FMath::Atan2(DeltaPos.Y, DeltaPos.X);

	FSlateDrawElement::MakeRotatedBox(
		DrawElementsList,
		ArrowLayerID,
		FPaintGeometry(ArrowDrawPos, ArrowImage->ImageSize * ZoomFactor, ZoomFactor),
		ArrowImage,
		ESlateDrawEffect::None,
		AngleInRadians,
		TOptional<FVector2f>(),
		FSlateDrawElement::RelativeToElement,
		Params.WireColor);
}

void FYSPlaybackConnectionDrawingPolicy::DrawPreviewConnector(const FGeometry& PinGeometry, const FVector2f& StartPoint, const FVector2f& EndPoint, UEdGraphPin* Pin)
{
	FConnectionParams Params;
	DetermineWiringStyle(Pin, nullptr, Params);

	// 선이 노드 안에서 시작하지 않도록 끌고 있는 쪽 끝점을 노드 경계에 붙인다.
	const FVector2f SeedPoint = (Pin->Direction == EGPD_Input) ? StartPoint : EndPoint;
	const FVector2f AdjustedSeedPoint = FGeometryHelper::FindClosestPointOnGeom(PinGeometry, SeedPoint);

	const FVector2f AdjustedStart = (Pin->Direction == EGPD_Input) ? StartPoint : AdjustedSeedPoint;
	const FVector2f AdjustedEnd = (Pin->Direction == EGPD_Input) ? AdjustedSeedPoint : EndPoint;

	DrawSplineWithArrow(AdjustedStart, AdjustedEnd, Params);
}

FVector2f FYSPlaybackConnectionDrawingPolicy::ComputeSplineTangent(const FVector2f& Start, const FVector2f& End) const
{
	return (End - Start).GetSafeNormal();
}


FConnectionDrawingPolicy* FYSPlaybackConnectionFactory::CreateConnectionPolicy(const UEdGraphSchema* Schema, int32 InBackLayerID, int32 InFrontLayerID,
                                                                               float ZoomFactor, const FSlateRect& InClippingRect,
                                                                               FSlateWindowElementList& InDrawElements, UEdGraph* InGraphObj) const
{
	if (Schema != nullptr && Schema->IsA<UYSPlaybackGraphSchema>())
	{
		return new FYSPlaybackConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, ZoomFactor, InClippingRect, InDrawElements, InGraphObj);
	}

	return nullptr;
}
