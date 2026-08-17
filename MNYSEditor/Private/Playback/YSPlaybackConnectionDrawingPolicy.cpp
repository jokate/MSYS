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
	// 상태 머신용 화살촉. 기본 스플라인 화살표보다 크고 방향이 또렷하다.
	ArrowImage = FAppStyle::GetBrush(TEXT("Graph.AnimStateNode.ConnectionArrow"));
}

void FYSPlaybackConnectionDrawingPolicy::DetermineWiringStyle(UEdGraphPin* OutputPin, UEdGraphPin* InputPin, FConnectionParams& Params)
{
	Params.AssociatedPin1 = OutputPin;
	Params.AssociatedPin2 = InputPin;
	Params.WireThickness = 1.5f;
	Params.WireColor = FLinearColor(0.72f, 0.72f, 0.72f);

	// 조건이 붙은 전환은 선까지 같은 색으로 물들인다. 전환 아이콘 색과 맞춘다.
	if (InputPin != nullptr)
	{
		if (const UYSPlaybackGraphNode_Transition* TransitionNode = Cast<UYSPlaybackGraphNode_Transition>(InputPin->GetOwningNode()))
		{
			if (TransitionNode->IsDangling())
			{
				Params.WireColor = FLinearColor(0.90f, 0.20f, 0.15f);
			}
			else if (TransitionNode->Edge.TransitionConditions.Num() > 0)
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
	// 시작 노드 → 상태 : 시작 노드는 핀 위젯을 그대로 쓰고 끝만 상태 노드로 잡는다.
	if (OutputPin->GetOwningNode()->IsA<UYSPlaybackGraphNode_Entry>())
	{
		StartWidgetGeometry = PinGeometries->Find(OutputPinWidget);

		if (const int32* StateIndex = NodeWidgetMap.Find(InputPin->GetOwningNode()))
		{
			EndWidgetGeometry = &(ArrangedNodes[*StateIndex]);
		}

		return;
	}

	// 상태 → 전환 : 이 연결을 그릴 때 양 끝을 [이전 상태]와 [다음 상태]로 바꿔치기한다.
	// 이래야 두 개의 연결이 화면에서 선 하나로 합쳐진다.
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

	// 전환 → 상태 연결은 위에서 이미 그려졌으므로 아무것도 하지 않는다.
	// 기하를 못 채우면 엔진이 그 연결을 건너뛴다 — 선이 두 번 겹쳐 그려지는 걸 막는다.
}

void FYSPlaybackConnectionDrawingPolicy::Draw(TMap<TSharedRef<SWidget>, FArrangedWidget>& InPinGeometries, FArrangedChildren& ArrangedNodes)
{
	// 노드 오브젝트로 배치 인덱스를 찾을 수 있게 표를 만든다. 위 바꿔치기가 이걸 쓴다.
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
	// 두 상자 사이의 최단 지점끼리 잇는다. 중심끼리 이으면 선이 상자를 파고든다.
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
	// A→B 와 B→A 가 같은 선 위에 겹치지 않도록 수직으로 살짝 민다.
	// 이 6픽셀이 왕복 전환을 두 줄로 보이게 만든다.
	const float LineSeparationAmount = 6.f * ZoomFactor;

	const FVector2f DeltaPos = EndAnchorPoint - StartAnchorPoint;
	const FVector2f UnitDelta = DeltaPos.GetSafeNormal();
	const FVector2f Normal = FVector2f(DeltaPos.Y, -DeltaPos.X).GetSafeNormal();

	const FVector2f DirectionBias = Normal * LineSeparationAmount;
	const FVector2f LengthBias = ArrowRadius.X * UnitDelta;

	const FVector2f StartPoint = StartAnchorPoint + DirectionBias + LengthBias;
	const FVector2f EndPoint = EndAnchorPoint + DirectionBias - LengthBias;

	// 화살촉과 선이 겹쳐 보이지 않도록 선을 조금 일찍 끊는다.
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

	// 끌고 있는 쪽 끝점을 노드 경계에 붙인다. 그래야 선이 노드 안에서 시작하지 않는다.
	const FVector2f SeedPoint = (Pin->Direction == EGPD_Input) ? StartPoint : EndPoint;
	const FVector2f AdjustedSeedPoint = FGeometryHelper::FindClosestPointOnGeom(PinGeometry, SeedPoint);

	const FVector2f AdjustedStart = (Pin->Direction == EGPD_Input) ? StartPoint : AdjustedSeedPoint;
	const FVector2f AdjustedEnd = (Pin->Direction == EGPD_Input) ? AdjustedSeedPoint : EndPoint;

	DrawSplineWithArrow(AdjustedStart, AdjustedEnd, Params);
}

FVector2f FYSPlaybackConnectionDrawingPolicy::ComputeSplineTangent(const FVector2f& Start, const FVector2f& End) const
{
	// 곡선이 아니라 직선으로 뽑는다. 상태 머신은 베지어보다 직선이 읽기 쉽다.
	return (End - Start).GetSafeNormal();
}


// ── 팩토리 ───────────────────────────────────────────────────────────────

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
