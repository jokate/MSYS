// Fill out your copyright notice in the Description page of Project Settings.


#include "Playback/YSPlaybackGraphCompiler.h"

#include "Ability/AbilityComponent/YSAbilityPlayback.h"
#include "Ability/AbilityComponent/YSPlaybackGraphAsset.h"
#include "EdGraph/EdGraph.h"
#include "Playback/YSPlaybackGraphNode.h"

namespace YSPlaybackCompile
{
	/** 시작 노드가 가리키는 상태. 없으면 nullptr. */
	static UYSPlaybackGraphNode_State* FindEntryState(const UEdGraph& Graph)
	{
		for (UEdGraphNode* Node : Graph.Nodes)
		{
			const UYSPlaybackGraphNode_Entry* EntryNode = Cast<UYSPlaybackGraphNode_Entry>(Node);

			if (EntryNode == nullptr)
			{
				continue;
			}

			const UEdGraphPin* OutputPin = EntryNode->GetOutputPin();

			if (OutputPin == nullptr || OutputPin->LinkedTo.Num() == 0)
			{
				return nullptr;
			}

			return Cast<UYSPlaybackGraphNode_State>(OutputPin->LinkedTo[0]->GetOwningNode());
		}

		return nullptr;
	}

	/** 이 상태에서 나가는 전환들. 우선순위 오름차순이며, 같으면 그래프 등록 순서를 따른다. */
	static void GatherOutgoingTransitions(const UYSPlaybackGraphNode_State& State, TArray<UYSPlaybackGraphNode_Transition*>& OutTransitions)
	{
		const UEdGraphPin* OutputPin = State.GetOutputPin();

		if (OutputPin == nullptr)
		{
			return;
		}

		for (const UEdGraphPin* LinkedPin : OutputPin->LinkedTo)
		{
			if (LinkedPin == nullptr)
			{
				continue;
			}

			if (UYSPlaybackGraphNode_Transition* Transition = Cast<UYSPlaybackGraphNode_Transition>(LinkedPin->GetOwningNode()))
			{
				OutTransitions.Add(Transition);
			}
		}

		// StableSort 를 쓴다. 우선순위가 같은 전환끼리 순서가 매번 뒤집히면
		// 컴파일할 때마다 에셋이 더러워져 소스 컨트롤이 시끄러워진다.
		OutTransitions.StableSort([](const UYSPlaybackGraphNode_Transition& A, const UYSPlaybackGraphNode_Transition& B)
		{
			return A.PriorityOrder < B.PriorityOrder;
		});
	}
}

void FYSPlaybackGraphCompiler::Compile(UYSPlaybackGraphAsset* Asset)
{
	if (IsValid(Asset) == false || Asset->EdGraph == nullptr)
	{
		return;
	}

	const UEdGraph& Graph = *Asset->EdGraph;

	// ── 1단계 : 상태 노드를 모으고 순서를 정한다 ──────────────────────────
	TArray<UYSPlaybackGraphNode_State*> StateNodes;

	for (UEdGraphNode* Node : Graph.Nodes)
	{
		if (UYSPlaybackGraphNode_State* StateNode = Cast<UYSPlaybackGraphNode_State>(Node))
		{
			if (StateNode->Playback != nullptr)
			{
				StateNodes.Add(StateNode);
			}
		}
	}

	// 시작 노드가 가리키는 상태를 맨 앞으로 끌어온다. 어빌리티는 0번부터 시작한다.
	if (UYSPlaybackGraphNode_State* EntryState = YSPlaybackCompile::FindEntryState(Graph))
	{
		const int32 EntryIndex = StateNodes.IndexOfByKey(EntryState);

		if (EntryIndex > 0)
		{
			StateNodes.Swap(0, EntryIndex);
		}
	}

	// ── 2단계 : 노드 → 인덱스 표를 만든다 ────────────────────────────────
	TMap<const UYSPlaybackGraphNode_State*, int32> NodeToIndex;
	NodeToIndex.Reserve(StateNodes.Num());

	for (int32 Index = 0; Index < StateNodes.Num(); ++Index)
	{
		NodeToIndex.Add(StateNodes[Index], Index);
	}

	// ── 3단계 : 플레이백을 에셋으로 복제한다 ─────────────────────────────
	// 이전 산출물은 버린다. GC 가 수거한다.
	Asset->Modify();
	Asset->Playbacks.Reset(StateNodes.Num());

	for (const UYSPlaybackGraphNode_State* StateNode : StateNodes)
	{
		UYSAbilityPlaybackBase* Compiled = DuplicateObject<UYSAbilityPlaybackBase>(StateNode->Playback, Asset);
		Asset->Playbacks.Add(Compiled);
	}

	// ── 4단계 : 전환을 엣지 배열로 굽는다 ────────────────────────────────
	for (int32 Index = 0; Index < StateNodes.Num(); ++Index)
	{
		TArray<UYSPlaybackGraphNode_Transition*> Transitions;
		YSPlaybackCompile::GatherOutgoingTransitions(*StateNodes[Index], Transitions);

		TArray<FYSPlaybackEdge> Edges;
		Edges.Reserve(Transitions.Num());

		for (const UYSPlaybackGraphNode_Transition* Transition : Transitions)
		{
			FYSPlaybackEdge Edge = Transition->BuildEdge();

			// 목적지 노드가 곧 전환의 뜻이다. 노드 프로퍼티에 적혀 있던 값은 여기서 덮어쓴다.
			// TriggerGameplayData 는 건드리지 않는다 — 이벤트 발행은 목적지와 무관한 속성이다.
			const UYSPlaybackGraphNode_Base* Target = Transition->GetTargetNode();

			if (const UYSPlaybackGraphNode_State* NextState = Cast<UYSPlaybackGraphNode_State>(Target))
			{
				const int32* NextIndex = NodeToIndex.Find(NextState);

				Edge.NextNodeIndex = (NextIndex != nullptr) ? *NextIndex : INDEX_NONE;
				Edge.bFireEventOnly = false;
			}
			else if (Target != nullptr && Target->IsA<UYSPlaybackGraphNode_Stay>())
			{
				// 전환하지 않는다. NextNodeIndex 는 읽히지 않지만 쓰레기 값을 남기지 않는다.
				Edge.NextNodeIndex = INDEX_NONE;
				Edge.bFireEventOnly = true;
			}
			else
			{
				// 종료 노드로 갔거나, 아무 데도 안 닿았다.
				// 둘 다 런타임 결과는 같지만 후자는 그래프에서 붉게 표시돼 실수임이 드러난다.
				Edge.NextNodeIndex = INDEX_NONE;
				Edge.bFireEventOnly = false;
			}

			Edges.Add(Edge);
		}

		Asset->Playbacks[Index]->Transitions = MoveTemp(Edges);
	}

	Asset->MarkPackageDirty();
}
