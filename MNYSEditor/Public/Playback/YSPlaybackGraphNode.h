// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphNode.h"
#include "Ability/AbilityComponent/YSAbilityPlayback.h"
#include "YSPlaybackGraphNode.generated.h"

class UYSAbilityPlaybackBase;

// 핀은 전부 같은 타입 하나다 — 이 그래프에는 데이터가 흐르지 않고 순서만 흐른다.
UCLASS(Abstract)
class UYSPlaybackGraphNode_Base : public UEdGraphNode
{
	GENERATED_BODY()

public:
	virtual UEdGraphPin* GetInputPin() const;
	virtual UEdGraphPin* GetOutputPin() const;

	// 엔진 기본 구현이 비어 있어 노드가 직접 넣지 않으면 삭제조차 메뉴에 안 뜬다.
	virtual void GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const override;
};


// 런타임 오브젝트를 노드가 직접 소유해서, 노드를 선택하면 Details 에 그 프로퍼티가 그대로 뜬다.
UCLASS()
class UYSPlaybackGraphNode_State : public UYSPlaybackGraphNode_Base
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Instanced, Category = "YS | Playback", meta = (DisplayName = "플레이백"))
	TObjectPtr<UYSAbilityPlaybackBase> Playback;

	UPROPERTY()
	TSubclassOf<UYSAbilityPlaybackBase> PlaybackClass;

	// 비어 있으면 플레이백 클래스 이름을 쓴다.
	UPROPERTY()
	FString StateName;

	//~ UEdGraphNode
	virtual void AllocateDefaultPins() override;
	virtual void PostPlacedNewNode() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FText GetTooltipText() const override;
	virtual bool GetCanRenameNode() const override { return true; }
	virtual void OnRenameNode(const FString& NewName) override;
	//~ End
};


// 체인의 시작점. 그래프에 하나만 있고 지울 수 없다.
UCLASS()
class UYSPlaybackGraphNode_Entry : public UYSPlaybackGraphNode_Base
{
	GENERATED_BODY()

public:
	//~ UEdGraphNode
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual bool CanUserDeleteNode() const override { return false; }
	virtual bool CanDuplicateNode() const override { return false; }
	//~ End
};


// 여기로 들어온 전환은 어빌리티를 종료시킨다. 연결 안 한 실수와 구분되고, 여러 개 놓아도 된다.
UCLASS()
class UYSPlaybackGraphNode_Exit : public UYSPlaybackGraphNode_Base
{
	GENERATED_BODY()

public:
	//~ UEdGraphNode
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	//~ End
};


// 전환하지 않고 유지한다. 입력·이벤트처럼 여러 번 나는 출력에만 의미가 있다 — 몽타주 완료에 붙이면 노드가 멈춘 채 남는다.
UCLASS()
class UYSPlaybackGraphNode_Stay : public UYSPlaybackGraphNode_Base
{
	GENERATED_BODY()

public:
	//~ UEdGraphNode
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	//~ End
};


// 화살표처럼 보이지만 두 상태 사이의 노드다 — 그래프 에디터의 선택 단위가 노드라서, 그래야 클릭 한 번에 전환 조건만 Details 에 뜬다.
// 같은 출력에 화살표가 여럿이면 우선순위대로 조건을 보고, 전부 실패하면 체인이 끝난다.
UCLASS()
class UYSPlaybackGraphNode_Transition : public UYSPlaybackGraphNode_Base
{
	GENERATED_BODY()

public:
	// 출발 상태의 Tasks 배열 인덱스. Task 순서를 바꾸면 이 값도 같이 고쳐야 한다.
	UPROPERTY(EditAnywhere, Category = "YS | Transition", meta = (DisplayName = "Task 인덱스", ClampMin = "0"))
	int32 TaskIndex = 0;

	// 몽타주 재생은 0 = 완료, 1 = 중단. 나머지는 0 하나뿐이다.
	UPROPERTY(EditAnywhere, Category = "YS | Transition", meta = (DisplayName = "출력 인덱스", ClampMin = "0"))
	int32 OutputIndex = 0;

	UPROPERTY(EditAnywhere, Category = "YS | Transition",
		meta = (DisplayName = "전환 조건", BaseStruct = "/Script/MNYS.YSPlaybackCondition", ExcludeBaseStruct))
	TArray<FInstancedStruct> TransitionConditions;

	// 같으면 연결한 순서.
	UPROPERTY(EditAnywhere, Category = "YS | Transition", meta = (DisplayName = "우선순위 (높을수록 먼저)"))
	int32 PriorityOrder = 0;

	// 목적지가 유지·종료여도 전환 직전에 돈다.
	UPROPERTY(EditAnywhere, Instanced, Category = "YS | Transition", meta = (DisplayName = "경로 행동"))
	TArray<TObjectPtr<UYSPlaybackAction>> Actions;

	// 인덱스가 어긋났으면 nullptr.
	UYSPlaybackTask* GetSourceTask() const;

	// 아니면 컴파일에서 빠지고 그래프에서 붉게 뜬다.
	bool HasValidOutput() const;

	void CreateConnections(UYSPlaybackGraphNode_Base* From, UYSPlaybackGraphNode_Base* To);

	UYSPlaybackGraphNode_Base* GetTargetNode() const;

	// 의도인지 실수인지 알 수 없으므로 그래프에서 붉게 표시한다.
	bool IsDangling() const { return GetTargetNode() == nullptr; }

	// 종료·유지로 가거나 연결이 없으면 nullptr.
	UYSPlaybackGraphNode_State* GetNextState() const;

	UYSPlaybackGraphNode_State* GetPreviousState() const;

	//~ UEdGraphNode
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FText GetTooltipText() const override;
	//~ End

	//~ UObject
	virtual void PostInitProperties() override;
	virtual void Serialize(FArchive& Ar) override;
	//~ End

private:
	// 1: 우선순위가 '높을수록 먼저'로 바뀜. 0 이면 로드할 때 부호를 뒤집는다.
	static constexpr int32 CurrentDataVersion = 1;

	UPROPERTY()
	int32 DataVersion = 0;
};
