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
}

void FYSPlaybackGraphCompiler::GatherOutgoingTransitions(const UYSPlaybackGraphNode_State& State, TArray<UYSPlaybackGraphNode_Transition*>& OutTransitions)
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

	// ── 4단계 : 화살표를 Task 출력의 경로로 굽는다 ──────────────────────
	for (int32 Index = 0; Index < StateNodes.Num(); ++Index)
	{
		UYSAbilityPlaybackBase* Compiled = Asset->Playbacks[Index];

		// 노드에 남아 있던 경로는 버린다. 그래프의 화살표만이 경로의 출처다.
		for (UYSPlaybackTask* Task : Compiled->Tasks)
		{
			if (IsValid(Task))
			{
				Task->ResetRoutes();
			}
		}

		TArray<UYSPlaybackGraphNode_Transition*> Transitions;
		GatherOutgoingTransitions(*StateNodes[Index], Transitions);

		for (const UYSPlaybackGraphNode_Transition* Transition : Transitions)
		{
			// 인덱스가 어긋난 화살표. 그래프에서 붉게 뜬다.
			if (Transition->HasValidOutput() == false || Compiled->Tasks.IsValidIndex(Transition->TaskIndex) == false)
			{
				continue;
			}

			UYSPlaybackTask* CompiledTask = Compiled->Tasks[Transition->TaskIndex];
			const UYSPlaybackGraphNode_Base* Target = Transition->GetTargetNode();

			FYSPlaybackRoute Route;
			Route.Conditions = Transition->TransitionConditions;

			// 종료 노드로 갔거나, 아무 데도 안 닿았다.
			// 둘 다 런타임 결과는 같지만 후자는 그래프에서 붉게 표시돼 실수임이 드러난다.
			Route.Target = EYSRouteTarget::End;

			if (const UYSPlaybackGraphNode_State* NextState = Cast<UYSPlaybackGraphNode_State>(Target))
			{
				if (const int32* NextIndex = NodeToIndex.Find(NextState))
				{
					Route.Target = EYSRouteTarget::Node;
					Route.NextNodeIndex = *NextIndex;
				}
			}
			else if (Target != nullptr && Target->IsA<UYSPlaybackGraphNode_Stay>())
			{
				Route.Target = EYSRouteTarget::Stay;

				if (CompiledTask->IsRepeatable() == false)
				{
					UE_LOG(LogTemp, Warning, TEXT("%s : 한 번만 나는 출력(%s)이 유지로 간다. 노드가 멈춘 채 남는다."),
						*Asset->GetPathName(), *CompiledTask->GetClass()->GetName());
				}
			}

			// 행동은 편집용 전환 노드가 소유한다. 그 노드는 쿠킹 때 사라지므로 산출물 쪽에 복제한다.
			for (const UYSPlaybackAction* Action : Transition->Actions)
			{
				if (Action != nullptr)
				{
					Route.Actions.Add(DuplicateObject<UYSPlaybackAction>(Action, CompiledTask));
				}
			}

			CompiledTask->AddRoute(Transition->OutputIndex, Route);
		}
	}

	// 산출물이 바뀌었음을 알린다. 이미 사본을 뜬 어빌리티가 이 번호를 보고 다시 뜬다.
	++Asset->CompileSerial;

	Asset->MarkPackageDirty();
}
