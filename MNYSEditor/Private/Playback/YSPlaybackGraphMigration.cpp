// Fill out your copyright notice in the Description page of Project Settings.


#include "Playback/YSPlaybackGraphMigration.h"

#include "Ability/YSGameplayAbility.h"
#include "Ability/AbilityComponent/YSAbilityPlayback.h"
#include "Ability/AbilityComponent/YSPlaybackGraphAsset.h"
#include "EdGraph/EdGraph.h"
#include "Playback/YSPlaybackGraphCompiler.h"
#include "Playback/YSPlaybackGraphNode.h"

namespace YSPlaybackMigration
{
	// 노드 배치 간격. 한 줄로 늘어놓고 나중에 사람이 옮긴다.
	static constexpr int32 StateSpacingX = 420;
	static constexpr int32 StateRowY = 0;

	static void ClearGraphExceptEntry(UEdGraph& Graph)
	{
		TArray<UEdGraphNode*> NodesToRemove;

		for (UEdGraphNode* Node : Graph.Nodes)
		{
			if (Node != nullptr && Node->IsA<UYSPlaybackGraphNode_Entry>() == false)
			{
				NodesToRemove.Add(Node);
			}
		}

		for (UEdGraphNode* Node : NodesToRemove)
		{
			Graph.RemoveNode(Node);
		}
	}

	static UYSPlaybackGraphNode_Entry* FindEntry(UEdGraph& Graph)
	{
		for (UEdGraphNode* Node : Graph.Nodes)
		{
			if (UYSPlaybackGraphNode_Entry* EntryNode = Cast<UYSPlaybackGraphNode_Entry>(Node))
			{
				return EntryNode;
			}
		}

		return nullptr;
	}
}

int32 FYSPlaybackGraphMigration::ImportFromSourceAbility(UYSPlaybackGraphAsset* Asset)
{
	if (IsValid(Asset) == false || Asset->EdGraph == nullptr)
	{
		return 0;
	}

	if (Asset->SourceAbility == nullptr)
	{
		return 0;
	}

	const UYSGameplayAbility* AbilityCDO = Asset->SourceAbility->GetDefaultObject<UYSGameplayAbility>();

	if (AbilityCDO == nullptr)
	{
		return 0;
	}

	const TArray<TObjectPtr<UYSAbilityPlaybackBase>>& SourcePlaybacks = AbilityCDO->GetLegacyPlaybacks();

	if (SourcePlaybacks.Num() == 0)
	{
		return 0;
	}

	UEdGraph& Graph = *Asset->EdGraph;

	Graph.Modify();
	YSPlaybackMigration::ClearGraphExceptEntry(Graph);

	// ── 1단계 : 플레이백마다 상태 노드를 만든다 ──────────────────────────
	// 인덱스가 곧 배열 위치이므로, 전환을 잇기 전에 전부 만들어 둬야 한다.
	TArray<UYSPlaybackGraphNode_State*> StateNodes;
	StateNodes.Reserve(SourcePlaybacks.Num());

	for (int32 Index = 0; Index < SourcePlaybacks.Num(); ++Index)
	{
		FGraphNodeCreator<UYSPlaybackGraphNode_State> NodeCreator(Graph);

		UYSPlaybackGraphNode_State* StateNode = NodeCreator.CreateNode(false);
		StateNode->NodePosX = Index * YSPlaybackMigration::StateSpacingX;
		StateNode->NodePosY = YSPlaybackMigration::StateRowY;

		// 어빌리티 CDO 가 소유한 원본을 그대로 참조하면 어빌리티를 지울 때 같이 날아간다.
		// 그래프 노드가 자기 사본을 갖는다.
		if (SourcePlaybacks[Index] != nullptr)
		{
			StateNode->Playback = DuplicateObject<UYSAbilityPlaybackBase>(SourcePlaybacks[Index], StateNode);
			StateNode->PlaybackClass = SourcePlaybacks[Index]->GetClass();
		}

		NodeCreator.Finalize();

		StateNodes.Add(StateNode);
	}

	// ── 2단계 : 시작 노드를 0번 상태에 잇는다 ────────────────────────────
	if (UYSPlaybackGraphNode_Entry* EntryNode = YSPlaybackMigration::FindEntry(Graph))
	{
		EntryNode->NodePosX = -300;
		EntryNode->NodePosY = YSPlaybackMigration::StateRowY;

		UEdGraphPin* EntryOutput = EntryNode->GetOutputPin();
		UEdGraphPin* FirstInput = StateNodes[0]->GetInputPin();

		if (EntryOutput != nullptr && FirstInput != nullptr)
		{
			EntryOutput->BreakAllPinLinks();
			EntryOutput->MakeLinkTo(FirstInput);
		}
	}

	// ── 3단계 : 엣지마다 전환 노드를 만든다 ──────────────────────────────
	for (int32 Index = 0; Index < SourcePlaybacks.Num(); ++Index)
	{
		if (SourcePlaybacks[Index] == nullptr)
		{
			continue;
		}

		const TArray<FYSPlaybackEdge>& Edges = SourcePlaybacks[Index]->Transitions;

		for (int32 EdgeIndex = 0; EdgeIndex < Edges.Num(); ++EdgeIndex)
		{
			const FYSPlaybackEdge& Edge = Edges[EdgeIndex];

			FGraphNodeCreator<UYSPlaybackGraphNode_Transition> NodeCreator(Graph);

			UYSPlaybackGraphNode_Transition* TransitionNode = NodeCreator.CreateNode(false);
			TransitionNode->Edge = Edge;

			// 배열 순서가 곧 우선순위였다. 그 순서를 숫자로 굳힌다.
			TransitionNode->PriorityOrder = EdgeIndex;

			NodeCreator.Finalize();

			// 목적지가 없으면(-1) 출력을 비워 둔다. 그게 체인 종료의 그림이다.
			UYSPlaybackGraphNode_State* NextState = StateNodes.IsValidIndex(Edge.NextNodeIndex) ? StateNodes[Edge.NextNodeIndex] : nullptr;

			if (NextState != nullptr)
			{
				TransitionNode->CreateConnections(StateNodes[Index], NextState);
			}
			else if (UEdGraphPin* FromOutput = StateNodes[Index]->GetOutputPin())
			{
				if (UEdGraphPin* TransitionInput = TransitionNode->GetInputPin())
				{
					FromOutput->MakeLinkTo(TransitionInput);
				}
			}

			// 위치는 출발 상태 아래쪽. 전환 위젯이 붙으면 2차 배치가 다시 잡는다.
			TransitionNode->NodePosX = StateNodes[Index]->NodePosX + (YSPlaybackMigration::StateSpacingX / 2);
			TransitionNode->NodePosY = YSPlaybackMigration::StateRowY + 180 + (EdgeIndex * 90);
		}
	}

	Graph.NotifyGraphChanged();

	// 옮긴 즉시 구워둔다. 사용자가 저장만 하고 닫아도 런타임 데이터가 맞는다.
	FYSPlaybackGraphCompiler::Compile(Asset);

	return StateNodes.Num();
}
