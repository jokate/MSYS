// Fill out your copyright notice in the Description page of Project Settings.


#include "Playback/SGraphNodeYSPlaybackTransition.h"

#include "ConnectionDrawingPolicy.h"
#include "Playback/YSPlaybackGraphNode.h"
#include "Styling/AppStyle.h"
#include "Styling/StyleColors.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

void SGraphNodeYSPlaybackTransition::Construct(const FArguments& InArgs, UYSPlaybackGraphNode_Transition* InNode)
{
	GraphNode = InNode;

	UpdateGraphNode();
}

void SGraphNodeYSPlaybackTransition::UpdateGraphNode()
{
	InputPins.Empty();
	OutputPins.Empty();

	RightNodeBox.Reset();
	LeftNodeBox.Reset();

	GetOrAddSlot(ENodeZone::Center)
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Center)
	[
		SNew(SOverlay)

		+ SOverlay::Slot()
		.Padding(2.f)
		[
			SNew(SImage)
			.Image(FAppStyle::GetBrush(TEXT("Graph.AnimTransitionNode.ColorSpill")))
			.ColorAndOpacity(this, &SGraphNodeYSPlaybackTransition::GetTransitionColor)
		]

		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(FMargin(8.f, 2.f))
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SImage)
				.Image(FAppStyle::GetBrush(TEXT("Graph.AnimTransitionNode.Icon")))
				.ColorAndOpacity(FStyleColors::Background)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(FMargin(4.f, 0.f, 0.f, 0.f))
			[
				SNew(STextBlock)
				.Text(this, &SGraphNodeYSPlaybackTransition::GetTransitionTitle)
				.ColorAndOpacity(FStyleColors::Background)
				.TextStyle(FAppStyle::Get(), TEXT("Graph.Node.NodeTitleExtraLines"))
			]
		]
	];
}

FText SGraphNodeYSPlaybackTransition::GetTransitionTitle() const
{
	if (GraphNode == nullptr)
	{
		return FText::GetEmpty();
	}

	return GraphNode->GetNodeTitle(ENodeTitleType::ListView);
}

FSlateColor SGraphNodeYSPlaybackTransition::GetTransitionColor() const
{
	const UYSPlaybackGraphNode_Transition* TransitionNode = Cast<UYSPlaybackGraphNode_Transition>(GraphNode);

	if (TransitionNode == nullptr)
	{
		return FLinearColor(0.55f, 0.55f, 0.55f);
	}

	// 안 닿은 전환과 Task 출력이 어긋나 컴파일에서 빠지는 전환은 실수이므로 붉게 띄운다.
	if (TransitionNode->IsDangling() || TransitionNode->HasValidOutput() == false)
	{
		return FLinearColor(0.90f, 0.20f, 0.15f);
	}

	return (TransitionNode->TransitionConditions.Num() > 0)
		? FLinearColor(0.95f, 0.70f, 0.25f)
		: FLinearColor(0.75f, 0.75f, 0.75f);
}

void SGraphNodeYSPlaybackTransition::PerformSecondPassLayout(const TMap<UObject*, TSharedRef<SNode>>& NodeToWidgetLookup) const
{
	UYSPlaybackGraphNode_Transition* TransitionNode = CastChecked<UYSPlaybackGraphNode_Transition>(GraphNode);

	UYSPlaybackGraphNode_State* PrevState = TransitionNode->GetPreviousState();
	UYSPlaybackGraphNode_Base* NextNode = TransitionNode->GetTargetNode();

	// 목적지가 종료·유지여도 선 위에 얹혀야 하므로 상태로 한정하지 않는다.
	if (PrevState == nullptr || NextNode == nullptr)
	{
		return;
	}

	const TSharedRef<SNode>* PrevWidget = NodeToWidgetLookup.Find(PrevState);
	const TSharedRef<SNode>* NextWidget = NodeToWidgetLookup.Find(NextNode);

	if (PrevWidget == nullptr || NextWidget == nullptr)
	{
		return;
	}

	const FGeometry StartGeom(FVector2D(PrevState->NodePosX, PrevState->NodePosY), FVector2D::ZeroVector, (*PrevWidget)->GetDesiredSize(), 1.f);
	const FGeometry EndGeom(FVector2D(NextNode->NodePosX, NextNode->NodePosY), FVector2D::ZeroVector, (*NextWidget)->GetDesiredSize(), 1.f);

	// 같은 두 상태를 잇는 전환이 여럿이면 겹치므로 몇 번째인지 센다.
	TArray<UYSPlaybackGraphNode_Transition*> SiblingTransitions;

	if (const UEdGraphPin* PrevOutput = PrevState->GetOutputPin())
	{
		for (const UEdGraphPin* LinkedPin : PrevOutput->LinkedTo)
		{
			UYSPlaybackGraphNode_Transition* Sibling = (LinkedPin != nullptr)
				? Cast<UYSPlaybackGraphNode_Transition>(LinkedPin->GetOwningNode())
				: nullptr;

			if (Sibling != nullptr && Sibling->GetTargetNode() == NextNode)
			{
				SiblingTransitions.Add(Sibling);
			}
		}
	}

	const int32 MyIndex = FMath::Max(SiblingTransitions.IndexOfByKey(TransitionNode), 0);
	const int32 SiblingCount = FMath::Max(SiblingTransitions.Num(), 1);

	// 자기 루프는 사이에 낄 공간이 없어 상태 위에 얹혀 읽히지도 눌리지도 않으므로 노드 위로 빼낸다.
	if (PrevState == NextNode)
	{
		const FVector2D NodeSize = (*PrevWidget)->GetDesiredSize();
		const FVector2D PillSize = GetDesiredSize();

		const double CenterX = PrevState->NodePosX + (NodeSize.X * 0.5) - (PillSize.X * 0.5);
		const double AboveY = PrevState->NodePosY - PillSize.Y - 28.0;

		const double StackOffset = MyIndex * (PillSize.Y + 6.0);

		GraphNode->NodePosX = static_cast<int32>(CenterX);
		GraphNode->NodePosY = static_cast<int32>(AboveY - StackOffset);

		return;
	}

	PositionBetweenTwoNodes(StartGeom, EndGeom, MyIndex, SiblingCount);
}

void SGraphNodeYSPlaybackTransition::PositionBetweenTwoNodes(const FGeometry& StartGeom, const FGeometry& EndGeom, int32 NodeIndex, int32 MaxNodes) const
{
	// 상자 중심끼리 이으면 선이 상자 안으로 파고들어 라벨이 묻히므로, 중간점에서 가장 가까운 지점을 쓴다.
	const FVector2D StartCenter = FGeometryHelper::CenterOf(StartGeom);
	const FVector2D EndCenter = FGeometryHelper::CenterOf(EndGeom);
	const FVector2D SeedPoint = (StartCenter + EndCenter) * 0.5f;

	const FVector2D StartAnchor = FGeometryHelper::FindClosestPointOnGeom(StartGeom, SeedPoint);
	const FVector2D EndAnchor = FGeometryHelper::FindClosestPointOnGeom(EndGeom, SeedPoint);

	FVector2D DeltaPos(EndAnchor - StartAnchor);

	// 자기 루프는 두 앵커가 같은 점이라 방향을 지어준다.
	if (DeltaPos.IsNearlyZero())
	{
		DeltaPos = FVector2D(10.f, 0.f);
	}

	// 선 위에 정확히 얹으면 선이 라벨을 관통해 수직으로 띄운다.
	const float LiftHeight = 24.f;
	const FVector2D Normal = FVector2D(DeltaPos.Y, -DeltaPos.X).GetSafeNormal();
	const FVector2D NewCenter = StartAnchor + (0.5f * DeltaPos) + (LiftHeight * Normal);

	const FVector2D DesiredNodeSize = GetDesiredSize();
	const FVector2D Direction = DeltaPos.GetSafeNormal();

	// 형제 전환들을 선 방향으로 한 칸씩 밀어 겹침을 푼다.
	const float Step = 1.f;
	const float StartOffset = -((MaxNodes - 1) * Step) * 0.5f;
	const float MyOffset = StartOffset + (NodeIndex * Step);

	const FVector2D NewCorner = NewCenter - (0.5f * DesiredNodeSize) + (Direction * MyOffset * DesiredNodeSize.X);

	GraphNode->NodePosX = static_cast<int32>(NewCorner.X);
	GraphNode->NodePosY = static_cast<int32>(NewCorner.Y);
}

int32 SGraphNodeYSPlaybackTransition::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
                                              FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	// 겹칠 때 라벨이 상자 뒤로 숨지 않게 상태 노드보다 앞에 그린다.
	return SGraphNode::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId + 100, InWidgetStyle, bParentEnabled);
}
