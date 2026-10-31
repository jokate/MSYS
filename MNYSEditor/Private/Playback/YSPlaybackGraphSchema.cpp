// Fill out your copyright notice in the Description page of Project Settings.


#include "Playback/YSPlaybackGraphSchema.h"

#include "Playback/YSPlaybackGraphNode.h"
#include "UObject/UObjectHash.h"

#define LOCTEXT_NAMESPACE "YSPlaybackGraphSchema"

const FName UYSPlaybackGraphSchema::PC_Transition(TEXT("Transition"));

void UYSPlaybackGraphSchema::CreateDefaultNodesForGraph(UEdGraph& Graph) const
{
	FGraphNodeCreator<UYSPlaybackGraphNode_Entry> NodeCreator(Graph);

	UYSPlaybackGraphNode_Entry* EntryNode = NodeCreator.CreateNode();
	EntryNode->NodePosX = -250;
	EntryNode->NodePosY = 0;

	NodeCreator.Finalize();

	SetNodeMetaData(EntryNode, FNodeMetadata::DefaultGraphNode);
}

void UYSPlaybackGraphSchema::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	TArray<UClass*> PlaybackClasses;
	GetDerivedClasses(UYSAbilityPlaybackBase::StaticClass(), PlaybackClasses, true);

	PlaybackClasses.Insert(UYSAbilityPlaybackBase::StaticClass(), 0);

	for (UClass* PlaybackClass : PlaybackClasses)
	{
		if (PlaybackClass == nullptr)
		{
			continue;
		}

		// HideDropDown 은 옛 에셋 로드용으로만 남긴 서브클래스다.
		if (PlaybackClass->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists | CLASS_HideDropDown))
		{
			continue;
		}

		const FText MenuDesc = PlaybackClass->GetDisplayNameText();

		const TSharedPtr<FEdGraphSchemaAction_NewNode> Action = MakeShared<FEdGraphSchemaAction_NewNode>(
			LOCTEXT("PlaybackCategory", "플레이백"),
			MenuDesc,
			PlaybackClass->GetToolTipText(),
			0);

		UYSPlaybackGraphNode_State* NodeTemplate = NewObject<UYSPlaybackGraphNode_State>(ContextMenuBuilder.OwnerOfTemporaries);
		NodeTemplate->PlaybackClass = PlaybackClass;

		Action->NodeTemplate = NodeTemplate;

		ContextMenuBuilder.AddAction(Action);
	}

	{
		const TSharedPtr<FEdGraphSchemaAction_NewNode> ExitAction = MakeShared<FEdGraphSchemaAction_NewNode>(
			LOCTEXT("FlowCategory", "흐름"),
			LOCTEXT("ExitMenuDesc", "종료"),
			LOCTEXT("ExitMenuTooltip", "여기로 들어온 전환은 어빌리티를 끝낸다."),
			0);

		ExitAction->NodeTemplate = NewObject<UYSPlaybackGraphNode_Exit>(ContextMenuBuilder.OwnerOfTemporaries);
		ContextMenuBuilder.AddAction(ExitAction);

		const TSharedPtr<FEdGraphSchemaAction_NewNode> StayAction = MakeShared<FEdGraphSchemaAction_NewNode>(
			LOCTEXT("FlowCategory", "흐름"),
			LOCTEXT("StayMenuDesc", "유지"),
			LOCTEXT("StayMenuTooltip", "전환하지 않고 현재 플레이백을 계속 재생한다. 화살표의 행동은 실행된다."),
			0);

		StayAction->NodeTemplate = NewObject<UYSPlaybackGraphNode_Stay>(ContextMenuBuilder.OwnerOfTemporaries);
		ContextMenuBuilder.AddAction(StayAction);
	}
}

const FPinConnectionResponse UYSPlaybackGraphSchema::CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const
{
	if (A == nullptr || B == nullptr)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("NoPin", "핀이 없다"));
	}

	if (A->GetOwningNode() == B->GetOwningNode())
	{
		// 상태 → 자기 자신은 전환 노드를 끼워야 하므로 여기서 막지 않고 아래 분기로 흘린다.
		if (A->Direction == B->Direction)
		{
			return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("SamePin", "같은 방향의 핀끼리는 이을 수 없다"));
		}
	}

	// 시작 노드는 전환을 끼지 않고, 반드시 상태에만 이어진다.
	if (A->GetOwningNode()->IsA<UYSPlaybackGraphNode_Entry>())
	{
		if (B->GetOwningNode()->IsA<UYSPlaybackGraphNode_State>() == false)
		{
			return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("EntryNeedsState", "시작은 상태 노드에만 연결할 수 있다"));
		}

		return FPinConnectionResponse(CONNECT_RESPONSE_BREAK_OTHERS_A, LOCTEXT("EntryLink", "시작 지점을 옮긴다"));
	}

	if (B->GetOwningNode()->IsA<UYSPlaybackGraphNode_Entry>())
	{
		if (A->GetOwningNode()->IsA<UYSPlaybackGraphNode_State>() == false)
		{
			return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("EntryNeedsStateB", "시작은 상태 노드에만 연결할 수 있다"));
		}

		return FPinConnectionResponse(CONNECT_RESPONSE_BREAK_OTHERS_B, LOCTEXT("EntryLinkB", "시작 지점을 옮긴다"));
	}

	const bool bAIsTransition = A->GetOwningNode()->IsA<UYSPlaybackGraphNode_Transition>();
	const bool bBIsTransition = B->GetOwningNode()->IsA<UYSPlaybackGraphNode_Transition>();

	// 전환은 입출력이 각각 하나뿐이라 새로 이으면 기존 연결을 끊는다.
	if (bAIsTransition)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_BREAK_OTHERS_A, LOCTEXT("ReplaceTransitionA", "기존 연결을 대체한다"));
	}

	if (bBIsTransition)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_BREAK_OTHERS_B, LOCTEXT("ReplaceTransitionB", "기존 연결을 대체한다"));
	}

	return FPinConnectionResponse(CONNECT_RESPONSE_MAKE_WITH_CONVERSION_NODE, LOCTEXT("CreateTransition", "전환을 만든다"));
}

bool UYSPlaybackGraphSchema::CreateAutomaticConversionNodeAndConnections(UEdGraphPin* A, UEdGraphPin* B) const
{
	UYSPlaybackGraphNode_Base* NodeA = Cast<UYSPlaybackGraphNode_Base>(A->GetOwningNode());
	UYSPlaybackGraphNode_Base* NodeB = Cast<UYSPlaybackGraphNode_Base>(B->GetOwningNode());

	if (NodeA == nullptr || NodeB == nullptr)
	{
		return false;
	}

	const FVector2f Location = (FVector2f(NodeA->NodePosX, NodeA->NodePosY) + FVector2f(NodeB->NodePosX, NodeB->NodePosY)) * 0.5f;

	UYSPlaybackGraphNode_Transition* TransitionNode = FEdGraphSchemaAction_NewNode::SpawnNodeFromTemplate<UYSPlaybackGraphNode_Transition>(
		NodeA->GetGraph(),
		NewObject<UYSPlaybackGraphNode_Transition>(),
		Location,
		false);

	if (TransitionNode == nullptr)
	{
		return false;
	}

	if (A->Direction == EGPD_Output)
	{
		TransitionNode->CreateConnections(NodeA, NodeB);
	}
	else
	{
		TransitionNode->CreateConnections(NodeB, NodeA);
	}

	return true;
}

FLinearColor UYSPlaybackGraphSchema::GetPinTypeColor(const FEdGraphPinType& PinType) const
{
	return FLinearColor(0.75f, 0.75f, 0.75f);
}

void UYSPlaybackGraphSchema::BreakPinLinks(UEdGraphPin& TargetPin, bool bSendsNodeNotifcation) const
{
	Super::BreakPinLinks(TargetPin, bSendsNodeNotifcation);

	// 끊긴 전환 노드는 일부러 남긴다 — 다시 이을 수 있어야 하고 컴파일에서 걸러진다.
}

#undef LOCTEXT_NAMESPACE
