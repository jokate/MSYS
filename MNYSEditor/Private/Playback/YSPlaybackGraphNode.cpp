// Fill out your copyright notice in the Description page of Project Settings.


#include "Playback/YSPlaybackGraphNode.h"

#include "Ability/AbilityComponent/YSPlaybackTask.h"
#include "Framework/Commands/GenericCommands.h"
#include "GraphEditorActions.h"
#include "Playback/YSPlaybackGraphSchema.h"
#include "ToolMenu.h"
#include "ToolMenuSection.h"

#define LOCTEXT_NAMESPACE "YSPlaybackGraphNode"

UEdGraphPin* UYSPlaybackGraphNode_Base::GetInputPin() const
{
	for (UEdGraphPin* Pin : Pins)
	{
		if (Pin != nullptr && Pin->Direction == EGPD_Input)
		{
			return Pin;
		}
	}

	return nullptr;
}

UEdGraphPin* UYSPlaybackGraphNode_Base::GetOutputPin() const
{
	for (UEdGraphPin* Pin : Pins)
	{
		if (Pin != nullptr && Pin->Direction == EGPD_Output)
		{
			return Pin;
		}
	}

	return nullptr;
}


void UYSPlaybackGraphNode_Base::GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
	if (Menu == nullptr || Context == nullptr || Context->Node == nullptr)
	{
		return;
	}

	// 핀을 직접 우클릭한 경우는 엔진이 알아서 핀 메뉴를 띄운다. 노드 동작만 얹는다.
	if (Context->Pin != nullptr)
	{
		return;
	}

	FToolMenuSection& Section = Menu->AddSection(TEXT("YSPlaybackNodeActions"), LOCTEXT("NodeActionsHeader", "노드 동작"));

	// 여기 올리는 커맨드는 툴킷의 GraphEditorCommands 에 매핑돼 있어야 눌린다.
	// 매핑 없이 올리면 항목은 뜨는데 회색으로 죽어 있다.
	Section.AddMenuEntry(FGenericCommands::Get().Delete);
	Section.AddMenuEntry(FGraphEditorCommands::Get().BreakNodeLinks);
}


// ── 상태 노드 ────────────────────────────────────────────────────────────

void UYSPlaybackGraphNode_State::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, UYSPlaybackGraphSchema::PC_Transition, TEXT("In"));
	CreatePin(EGPD_Output, UYSPlaybackGraphSchema::PC_Transition, TEXT("Out"));
}

void UYSPlaybackGraphNode_State::PostPlacedNewNode()
{
	Super::PostPlacedNewNode();

	// 붙여넣기로 온 노드는 이미 인스턴스를 갖고 있다. 비어 있을 때만 만든다.
	if (Playback == nullptr && PlaybackClass != nullptr)
	{
		Playback = NewObject<UYSAbilityPlaybackBase>(this, PlaybackClass, NAME_None, RF_Transactional);
	}
}

FText UYSPlaybackGraphNode_State::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (StateName.IsEmpty() == false)
	{
		return FText::FromString(StateName);
	}

	if (Playback == nullptr)
	{
		return LOCTEXT("EmptyState", "빈 플레이백");
	}

	return Playback->GetClass()->GetDisplayNameText();
}

void UYSPlaybackGraphNode_State::OnRenameNode(const FString& NewName)
{
	Modify();
	StateName = NewName;
}

FLinearColor UYSPlaybackGraphNode_State::GetNodeTitleColor() const
{
	return FLinearColor(0.12f, 0.35f, 0.60f);
}

FText UYSPlaybackGraphNode_State::GetTooltipText() const
{
	return LOCTEXT("StateTooltip", "플레이백 노드. 몽타주 또는 시퀀스를 재생한다.");
}


// ── 진입 노드 ────────────────────────────────────────────────────────────

void UYSPlaybackGraphNode_Entry::AllocateDefaultPins()
{
	CreatePin(EGPD_Output, UYSPlaybackGraphSchema::PC_Transition, TEXT("Entry"));
}

FText UYSPlaybackGraphNode_Entry::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("EntryTitle", "시작");
}

FLinearColor UYSPlaybackGraphNode_Entry::GetNodeTitleColor() const
{
	return FLinearColor(0.10f, 0.45f, 0.15f);
}


// ── 종료 / 유지 노드 ─────────────────────────────────────────────────────

void UYSPlaybackGraphNode_Exit::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, UYSPlaybackGraphSchema::PC_Transition, TEXT("In"));
}

FText UYSPlaybackGraphNode_Exit::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("ExitTitle", "종료");
}

FText UYSPlaybackGraphNode_Exit::GetTooltipText() const
{
	return LOCTEXT("ExitTooltip", "여기로 들어온 전환은 체인을 끝낸다. 어빌리티가 EndAbility 된다.");
}

void UYSPlaybackGraphNode_Stay::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, UYSPlaybackGraphSchema::PC_Transition, TEXT("In"));
}

FText UYSPlaybackGraphNode_Stay::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("StayTitle", "유지");
}

FText UYSPlaybackGraphNode_Stay::GetTooltipText() const
{
	return LOCTEXT("StayTooltip",
		"전환하지 않고 현재 플레이백을 계속 재생한다.\n"
		"화살표의 경로 행동은 그대로 실행된다. 입력·이벤트 대기처럼 여러 번 나는 출력에만 쓴다.");
}


// ── 전환 노드 ────────────────────────────────────────────────────────────

void UYSPlaybackGraphNode_Transition::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, UYSPlaybackGraphSchema::PC_Transition, TEXT("In"));
	CreatePin(EGPD_Output, UYSPlaybackGraphSchema::PC_Transition, TEXT("Out"));
}

UYSPlaybackTask* UYSPlaybackGraphNode_Transition::GetSourceTask() const
{
	const UYSPlaybackGraphNode_State* PrevState = GetPreviousState();

	if (PrevState == nullptr || PrevState->Playback == nullptr)
	{
		return nullptr;
	}

	const TArray<TObjectPtr<UYSPlaybackTask>>& Tasks = PrevState->Playback->Tasks;

	return Tasks.IsValidIndex(TaskIndex) ? Tasks[TaskIndex].Get() : nullptr;
}

bool UYSPlaybackGraphNode_Transition::HasValidOutput() const
{
	const UYSPlaybackTask* Task = GetSourceTask();

	return Task != nullptr && OutputIndex >= 0 && OutputIndex < Task->GetOutputCount();
}

void UYSPlaybackGraphNode_Transition::CreateConnections(UYSPlaybackGraphNode_Base* From, UYSPlaybackGraphNode_Base* To)
{
	if (From == nullptr || To == nullptr)
	{
		return;
	}

	UEdGraphPin* MyInput = GetInputPin();
	UEdGraphPin* MyOutput = GetOutputPin();

	if (MyInput == nullptr || MyOutput == nullptr)
	{
		return;
	}

	// 이 전환은 딱 한 쌍만 잇는다. 남아 있던 연결은 끊는다.
	MyInput->Modify();
	MyInput->BreakAllPinLinks();

	MyOutput->Modify();
	MyOutput->BreakAllPinLinks();

	if (UEdGraphPin* FromOutput = From->GetOutputPin())
	{
		FromOutput->Modify();
		FromOutput->MakeLinkTo(MyInput);
	}

	if (UEdGraphPin* ToInput = To->GetInputPin())
	{
		ToInput->Modify();
		MyOutput->MakeLinkTo(ToInput);
	}
}

UYSPlaybackGraphNode_Base* UYSPlaybackGraphNode_Transition::GetTargetNode() const
{
	const UEdGraphPin* MyOutput = GetOutputPin();

	if (MyOutput == nullptr || MyOutput->LinkedTo.Num() == 0)
	{
		return nullptr;
	}

	return Cast<UYSPlaybackGraphNode_Base>(MyOutput->LinkedTo[0]->GetOwningNode());
}

UYSPlaybackGraphNode_State* UYSPlaybackGraphNode_Transition::GetNextState() const
{
	return Cast<UYSPlaybackGraphNode_State>(GetTargetNode());
}

UYSPlaybackGraphNode_State* UYSPlaybackGraphNode_Transition::GetPreviousState() const
{
	const UEdGraphPin* MyInput = GetInputPin();

	if (MyInput == nullptr || MyInput->LinkedTo.Num() == 0)
	{
		return nullptr;
	}

	return Cast<UYSPlaybackGraphNode_State>(MyInput->LinkedTo[0]->GetOwningNode());
}

FText UYSPlaybackGraphNode_Transition::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	// 어느 Task 의 어느 출력인지가 곧 이 전환의 정체다. 같은 종류의 Task 가 여럿일 수 있어 인덱스를 붙인다.
	const UYSPlaybackTask* Task = GetSourceTask();

	if (Task == nullptr || HasValidOutput() == false)
	{
		return LOCTEXT("InvalidOutput", "출력 없음");
	}

	const FText OutputText = FText::Format(
		LOCTEXT("TransitionTitle", "[{0}] {1} · {2}"),
		FText::AsNumber(TaskIndex),
		Task->GetClass()->GetDisplayNameText(),
		Task->GetOutputDisplayName(OutputIndex));

	// 행동이 달린 화살표는 선 위에서 티가 나야 한다. 목적지만 봐서는 알 수 없다.
	if (Actions.Num() > 0)
	{
		return FText::Format(LOCTEXT("TransitionTitleWithActions", "{0}  ▸ 행동 {1}"), OutputText, FText::AsNumber(Actions.Num()));
	}

	return OutputText;
}

FLinearColor UYSPlaybackGraphNode_Transition::GetNodeTitleColor() const
{
	return FLinearColor(0.45f, 0.30f, 0.10f);
}

FText UYSPlaybackGraphNode_Transition::GetTooltipText() const
{
	return LOCTEXT("TransitionTooltip", "전환 노드. 출발 상태의 Task 출력 하나와 전환 조건을 담는다.");
}

#undef LOCTEXT_NAMESPACE
