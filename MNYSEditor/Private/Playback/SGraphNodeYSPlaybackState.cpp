// Fill out your copyright notice in the Description page of Project Settings.


#include "Playback/SGraphNodeYSPlaybackState.h"

#include "Playback/YSPlaybackGraphNode.h"
#include "ScopedTransaction.h"
#include "SGraphPanel.h"
#include "Styling/AppStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "SGraphNodeYSPlaybackState"

// ── 노드 전체를 덮는 핀 ──────────────────────────────────────────────────

void SYSPlaybackStatePin::Construct(const FArguments& InArgs, UEdGraphPin* InPin)
{
	SetCursor(EMouseCursor::Default);

	bShowLabel = true;
	GraphPinObj = InPin;

	check(GraphPinObj != nullptr);

	SBorder::Construct(SBorder::FArguments()
		.BorderImage(this, &SYSPlaybackStatePin::GetPinBorder)
		.BorderBackgroundColor(this, &SYSPlaybackStatePin::GetPinColor)
		.OnMouseButtonDown(this, &SYSPlaybackStatePin::OnPinMouseDown)
		.Cursor(this, &SYSPlaybackStatePin::GetPinCursor));
}

TSharedRef<SWidget> SYSPlaybackStatePin::GetDefaultValueWidget()
{
	return SNew(STextBlock);
}

const FSlateBrush* SYSPlaybackStatePin::GetPinBorder() const
{
	return IsHovered()
		? FAppStyle::GetBrush(TEXT("Graph.AnimStateNode.Pin.BackgroundHovered"))
		: FAppStyle::GetBrush(TEXT("Graph.AnimStateNode.Pin.Background"));
}


// ── 상태 노드 ────────────────────────────────────────────────────────────

void SGraphNodeYSPlaybackState::Construct(const FArguments& InArgs, UYSPlaybackGraphNode_Base* InNode)
{
	GraphNode = InNode;

	SetCursor(EMouseCursor::CardinalCross);

	UpdateGraphNode();
}

const FSlateBrush* SGraphNodeYSPlaybackState::GetBodyBrush() const
{
	return FAppStyle::GetBrush(TEXT("Graph.AnimStateNode.Body"));
}

const FSlateBrush* SGraphNodeYSPlaybackState::GetSpillBrush() const
{
	return FAppStyle::GetBrush(TEXT("Graph.AnimStateNode.ColorSpill"));
}

const FSlateBrush* SGraphNodeYSPlaybackState::GetIconBrush() const
{
	return FAppStyle::GetBrush(TEXT("Graph.StateNode.Icon"));
}

FLinearColor SGraphNodeYSPlaybackState::GetSpillColor() const
{
	return FLinearColor(0.60f, 0.60f, 0.60f);
}

FSlateColor SGraphNodeYSPlaybackState::GetBorderBackgroundColor() const
{
	// 선택됐을 때만 밝게. 나머지는 얌전한 회색이라 상태 이름이 튀어 보인다.
	const FLinearColor InactiveColor(0.08f, 0.08f, 0.08f);
	const FLinearColor ActiveColor(1.00f, 0.60f, 0.35f);

	const TSharedPtr<SGraphPanel> OwnerPanel = OwnerGraphPanelPtr.Pin();
	const bool bSelected = OwnerPanel.IsValid() && OwnerPanel->SelectionManager.IsNodeSelected(GraphNode);

	return bSelected ? ActiveColor : InactiveColor;
}

void SGraphNodeYSPlaybackState::UpdateGraphNode()
{
	InputPins.Empty();
	OutputPins.Empty();

	RightNodeBox.Reset();
	LeftNodeBox.Reset();

	// 노드 자체는 히트 테스트에서 빠진다. 클릭은 위에 덮인 핀이 받는다.
	SetVisibility(EVisibility::SelfHitTestInvisible);

	ContentScale.Bind(this, &SGraphNode::GetContentScale);

	GetOrAddSlot(ENodeZone::Center)
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Center)
	[
		SNew(SOverlay)
		.Visibility(EVisibility::SelfHitTestInvisible)

		+ SOverlay::Slot()
		.Padding(2.f)
		[
			SNew(SBorder)
			.BorderImage(GetBodyBrush())
			.Padding(0.f)
			.BorderBackgroundColor(this, &SGraphNodeYSPlaybackState::GetBorderBackgroundColor)
			[
				SNew(SOverlay)

				// 핀 영역 — 본체 전체를 덮는다.
				+ SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SAssignNew(RightNodeBox, SVerticalBox)
					+ SVerticalBox::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					.FillHeight(1.f)
					[
						SAssignNew(PinOverlay, SOverlay)
					]
				]

				// 이름 영역
				+ SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.Padding(12.f)
				[
					SNew(SBorder)
					.BorderImage(GetSpillBrush())
					.BorderBackgroundColor(GetSpillColor())
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.Padding(FMargin(6.f, 3.f))
					[
						SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(FMargin(2.f, 0.f, 6.f, 0.f))
						[
							SNew(SImage)
							.Image(GetIconBrush())
						]

						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						[
							SAssignNew(InlineEditableText, SInlineEditableTextBlock)
							.Style(FAppStyle::Get(), TEXT("Graph.AnimStateNode.NodeTitleInlineEditableText"))
							.Text_Lambda([this]()
							{
								return (GraphNode != nullptr) ? GraphNode->GetNodeTitle(ENodeTitleType::EditableTitle) : FText::GetEmpty();
							})
							.OnVerifyTextChanged(this, &SGraphNodeYSPlaybackState::OnVerifyNameTextChanged)
							.OnTextCommitted(this, &SGraphNodeYSPlaybackState::OnNameTextCommited)
							.IsReadOnly(this, &SGraphNodeYSPlaybackState::IsNameReadOnly)
							.IsSelected(this, &SGraphNodeYSPlaybackState::IsSelectedExclusively)
						]
					]
				]
			]
		]
	];

	CreatePinWidgets();
}

void SGraphNodeYSPlaybackState::CreatePinWidgets()
{
	const UYSPlaybackGraphNode_Base* Node = CastChecked<UYSPlaybackGraphNode_Base>(GraphNode);

	// 핀 위젯은 하나만 만든다. 두 개를 겹쳐두면 어느 쪽에서 드래그가 시작되는지 애매해진다.
	// 출력이 있으면 출력을(선을 끌어내는 쪽), 없으면 입력을(종료·유지처럼 받기만 하는 쪽) 쓴다.
	UEdGraphPin* PinToShow = Node->GetOutputPin();

	if (PinToShow == nullptr)
	{
		PinToShow = Node->GetInputPin();
	}

	if (PinToShow != nullptr && PinToShow->bHidden == false)
	{
		AddPin(SNew(SYSPlaybackStatePin, PinToShow));
	}
}

void SGraphNodeYSPlaybackState::AddPin(const TSharedRef<SGraphPin>& PinToAdd)
{
	PinToAdd->SetOwner(SharedThis(this));

	PinOverlay->AddSlot()
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	[
		PinToAdd
	];

	switch (PinToAdd->GetPinObj()->Direction)
	{
	case EGPD_Input:
		InputPins.Add(PinToAdd);
		break;

	case EGPD_Output:
		OutputPins.Add(PinToAdd);
		break;

	default:
		break;
	}
}

bool SGraphNodeYSPlaybackState::OnVerifyNameTextChanged(const FText& InText, FText& OutErrorMessage)
{
	if (InText.ToString().Len() > 64)
	{
		OutErrorMessage = LOCTEXT("NameTooLong", "이름이 너무 길다");
		return false;
	}

	return true;
}

void SGraphNodeYSPlaybackState::OnNameTextCommited(const FText& InText, ETextCommit::Type CommitInfo)
{
	if (GraphNode == nullptr)
	{
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("RenameNode", "노드 이름 변경"));

	GraphNode->OnRenameNode(InText.ToString());
}


// ── 시작 노드 ────────────────────────────────────────────────────────────

void SGraphNodeYSPlaybackEntry::Construct(const FArguments& InArgs, UYSPlaybackGraphNode_Base* InNode)
{
	SGraphNodeYSPlaybackState::Construct(SGraphNodeYSPlaybackState::FArguments(), InNode);
}

const FSlateBrush* SGraphNodeYSPlaybackEntry::GetBodyBrush() const
{
	return FAppStyle::GetBrush(TEXT("Graph.AnimConduitNode.Body"));
}

const FSlateBrush* SGraphNodeYSPlaybackEntry::GetSpillBrush() const
{
	return FAppStyle::GetBrush(TEXT("Graph.AnimConduitNode.ColorSpill"));
}

const FSlateBrush* SGraphNodeYSPlaybackEntry::GetIconBrush() const
{
	return FAppStyle::GetBrush(TEXT("Graph.EntryNode.Icon"));
}

FLinearColor SGraphNodeYSPlaybackEntry::GetSpillColor() const
{
	return FLinearColor(0.35f, 0.75f, 0.40f);
}


// ── 종료 노드 ────────────────────────────────────────────────────────────

void SGraphNodeYSPlaybackExit::Construct(const FArguments& InArgs, UYSPlaybackGraphNode_Base* InNode)
{
	SGraphNodeYSPlaybackState::Construct(SGraphNodeYSPlaybackState::FArguments(), InNode);
}

const FSlateBrush* SGraphNodeYSPlaybackExit::GetBodyBrush() const
{
	return FAppStyle::GetBrush(TEXT("Graph.AnimConduitNode.Body"));
}

const FSlateBrush* SGraphNodeYSPlaybackExit::GetSpillBrush() const
{
	return FAppStyle::GetBrush(TEXT("Graph.AnimConduitNode.ColorSpill"));
}

const FSlateBrush* SGraphNodeYSPlaybackExit::GetIconBrush() const
{
	return FAppStyle::GetBrush(TEXT("Graph.ResultNode.Icon"));
}

FLinearColor SGraphNodeYSPlaybackExit::GetSpillColor() const
{
	return FLinearColor(0.80f, 0.25f, 0.20f);
}


// ── 유지 노드 ────────────────────────────────────────────────────────────

void SGraphNodeYSPlaybackStay::Construct(const FArguments& InArgs, UYSPlaybackGraphNode_Base* InNode)
{
	SGraphNodeYSPlaybackState::Construct(SGraphNodeYSPlaybackState::FArguments(), InNode);
}

const FSlateBrush* SGraphNodeYSPlaybackStay::GetBodyBrush() const
{
	return FAppStyle::GetBrush(TEXT("Graph.AnimConduitNode.Body"));
}

const FSlateBrush* SGraphNodeYSPlaybackStay::GetSpillBrush() const
{
	return FAppStyle::GetBrush(TEXT("Graph.AnimConduitNode.ColorSpill"));
}

const FSlateBrush* SGraphNodeYSPlaybackStay::GetIconBrush() const
{
	return FAppStyle::GetBrush(TEXT("Graph.StateNode.Icon"));
}

FLinearColor SGraphNodeYSPlaybackStay::GetSpillColor() const
{
	return FLinearColor(0.30f, 0.55f, 0.80f);
}

#undef LOCTEXT_NAMESPACE
