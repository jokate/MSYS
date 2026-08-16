// Fill out your copyright notice in the Description page of Project Settings.


#include "Playback/YSPlaybackGraphSchema.h"

#include "Playback/YSPlaybackGraphNode.h"
#include "UObject/UObjectHash.h"

#define LOCTEXT_NAMESPACE "YSPlaybackGraphSchema"

const FName UYSPlaybackGraphSchema::PC_Transition(TEXT("Transition"));

void UYSPlaybackGraphSchema::CreateDefaultNodesForGraph(UEdGraph& Graph) const
{
	// 시작점은 그래프와 수명이 같다. 사용자가 만들지도 지우지도 않는다.
	FGraphNodeCreator<UYSPlaybackGraphNode_Entry> NodeCreator(Graph);

	UYSPlaybackGraphNode_Entry* EntryNode = NodeCreator.CreateNode();
	EntryNode->NodePosX = -250;
	EntryNode->NodePosY = 0;

	NodeCreator.Finalize();

	SetNodeMetaData(EntryNode, FNodeMetadata::DefaultGraphNode);
}

void UYSPlaybackGraphSchema::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	// 플레이백 파생 클래스를 전부 생성 메뉴에 올린다.
	// 새 파생 클래스를 만들면 여기 손대지 않아도 메뉴에 나타난다.
	TArray<UClass*> PlaybackClasses;
	GetDerivedClasses(UYSAbilityPlaybackBase::StaticClass(), PlaybackClasses, true);

	PlaybackClasses.Insert(UYSAbilityPlaybackBase::StaticClass(), 0);

	for (UClass* PlaybackClass : PlaybackClasses)
	{
		if (PlaybackClass == nullptr)
		{
			continue;
		}

		if (PlaybackClass->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists))
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
}

const FPinConnectionResponse UYSPlaybackGraphSchema::CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const
{
	if (A == nullptr || B == nullptr)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("NoPin", "핀이 없다"));
	}

	if (A->GetOwningNode() == B->GetOwningNode())
	{
		// 자기 자신으로 도는 연사 루프는 필요하다. 다만 상태 → 자기 자신은
		// 전환 노드를 끼워야 하므로 여기서 막지 않고 아래 분기로 흘린다.
		if (A->Direction == B->Direction)
		{
			return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("SamePin", "같은 방향의 핀끼리는 이을 수 없다"));
		}
	}

	// 시작 노드는 전환을 끼지 않는다. 어디서 시작하는지는 조건이 아니라 사실이다.
	if (A->GetOwningNode()->IsA<UYSPlaybackGraphNode_Entry>())
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_BREAK_OTHERS_A, LOCTEXT("EntryLink", "시작 지점을 옮긴다"));
	}

	if (B->GetOwningNode()->IsA<UYSPlaybackGraphNode_Entry>())
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_BREAK_OTHERS_B, LOCTEXT("EntryLinkB", "시작 지점을 옮긴다"));
	}

	const bool bAIsTransition = A->GetOwningNode()->IsA<UYSPlaybackGraphNode_Transition>();
	const bool bBIsTransition = B->GetOwningNode()->IsA<UYSPlaybackGraphNode_Transition>();

	// 전환은 입출력이 각각 하나뿐이다. 새로 이으면 기존 연결을 끊는다.
	if (bAIsTransition)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_BREAK_OTHERS_A, LOCTEXT("ReplaceTransitionA", "기존 연결을 대체한다"));
	}

	if (bBIsTransition)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_BREAK_OTHERS_B, LOCTEXT("ReplaceTransitionB", "기존 연결을 대체한다"));
	}

	// 상태끼리 이었다 — 사이에 전환 노드를 만들어 끼운다.
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

	// 두 상태의 중간 지점에 놓는다. 전용 위젯이 붙기 전까지는 이 좌표가 그대로 보인다.
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

	// 드래그 방향과 무관하게 출력 → 입력이 되도록 정렬한다.
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

	// 연결이 끊긴 전환 노드는 아무 데도 닿지 않는 유령이 된다.
	// 지금은 남겨둔다 — 다시 이을 수 있어야 하고, 컴파일에서 걸러내는 편이 낫다.
}

#undef LOCTEXT_NAMESPACE
