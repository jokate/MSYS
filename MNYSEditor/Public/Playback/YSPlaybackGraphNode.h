// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphNode.h"
#include "Ability/AbilityComponent/YSAbilityPlayback.h"
#include "YSPlaybackGraphNode.generated.h"

class UYSAbilityPlaybackBase;

/**
 * 플레이백 그래프 노드의 공통 베이스.
 *
 * 핀은 전부 같은 타입 하나다 — 이 그래프에는 데이터가 흐르지 않고 순서만 흐른다.
 */
UCLASS(Abstract)
class UYSPlaybackGraphNode_Base : public UEdGraphNode
{
	GENERATED_BODY()

public:
	virtual UEdGraphPin* GetInputPin() const;
	virtual UEdGraphPin* GetOutputPin() const;

	/**
	 * 우클릭 메뉴를 채운다.
	 *
	 * 엔진 기본 구현이 비어 있어서(EdGraphSchema.cpp:1209) 노드가 직접 넣지 않으면
	 * 삭제조차 메뉴에 안 뜬다. USoundCueGraphNode 가 같은 자리에서 같은 일을 한다.
	 */
	virtual void GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const override;
};


/**
 * 플레이백 하나 = 상태 노드 하나.
 *
 * 런타임 오브젝트(UYSAbilityPlaybackBase)를 노드가 직접 소유한다.
 * 그래서 노드를 선택하면 Details 에 지금까지 쓰던 그 프로퍼티가 그대로 뜬다.
 * USoundCue 의 그래프 노드가 USoundNode 를 들고 있는 것과 같은 구조다.
 */
UCLASS()
class UYSPlaybackGraphNode_State : public UYSPlaybackGraphNode_Base
{
	GENERATED_BODY()

public:
	/** 이 노드가 재생할 플레이백. 생성 메뉴에서 고른 클래스로 만들어진다. */
	UPROPERTY(EditAnywhere, Instanced, Category = "YS | Playback", meta = (DisplayName = "플레이백"))
	TObjectPtr<UYSAbilityPlaybackBase> Playback;

	/** 노드 생성 직후 인스턴스를 만들 클래스. 생성 메뉴가 채워준다. */
	UPROPERTY()
	TSubclassOf<UYSAbilityPlaybackBase> PlaybackClass;

	/**
	 * 화면에 뜨는 이름. 비어 있으면 플레이백 클래스 이름을 쓴다.
	 * 클래스 이름만으로는 같은 종류가 여럿일 때 구분이 안 된다 — "돌진", "마무리" 처럼 붙인다.
	 */
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


/** 체인의 시작점. 그래프에 하나만 있고 지울 수 없다. */
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


/**
 * 체인의 끝. 여기로 들어온 전환은 어빌리티를 종료시킨다.
 *
 * 예전에는 NextNodeIndex = -1 이 그 뜻이었는데, 그건 그래프에서 '아무 데도 안 닿은 선'과
 * 똑같이 보였다. 종료를 노드로 만들면 "여기서 끝난다"가 의도임이 드러나고,
 * 진짜 실수(연결 안 함)와 구분된다.
 *
 * 여러 개 놓아도 된다. 선이 길어지는 것보다 종료 노드가 여럿인 편이 읽기 쉽다.
 */
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


/**
 * 전환하지 않고 현재 플레이백을 유지한다. 경로 목적지 Stay 에 대응한다.
 *
 * 화살표의 경로 행동은 그대로 실행되고, 출발 Task 는 계속 대기한다.
 * 입력·이벤트처럼 여러 번 나는 출력에만 의미가 있다 — 몽타주 완료에 붙이면 노드가 멈춘 채 남는다.
 */
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


/**
 * 전환 = 노드.
 *
 * 화면에서는 화살표로 보이지만 실제로는 상태 A 와 상태 B 사이에 낀 노드다.
 * 이렇게 두는 이유는 하나다 — 그래프 에디터의 선택 단위가 노드라서,
 * 전환을 노드로 만들어야 클릭 한 번에 그 전환의 조건만 Details 에 띄울 수 있다.
 *
 * 엔진의 UAnimStateTransitionNode 가 같은 이유로 같은 선택을 했다.
 *
 * 화살표 하나 = 출발 상태의 Task 출력 하나에서 갈 수 있는 목적지 후보 하나.
 * 같은 출력에 화살표가 여럿이면 우선순위대로 조건을 보고, 전부 실패하면 체인이 끝난다.
 */
UCLASS()
class UYSPlaybackGraphNode_Transition : public UYSPlaybackGraphNode_Base
{
	GENERATED_BODY()

public:
	/** 출발 상태의 Tasks 배열 인덱스. Task 순서를 바꾸면 이 값도 같이 고쳐야 한다. */
	UPROPERTY(EditAnywhere, Category = "YS | Transition", meta = (DisplayName = "Task 인덱스", ClampMin = "0"))
	int32 TaskIndex = 0;

	/** 그 Task 의 출력. 몽타주 재생은 0 = 완료, 1 = 중단. 나머지는 0 하나뿐이다. */
	UPROPERTY(EditAnywhere, Category = "YS | Transition", meta = (DisplayName = "출력 인덱스", ClampMin = "0"))
	int32 OutputIndex = 0;

	UPROPERTY(EditAnywhere, Category = "YS | Transition",
		meta = (DisplayName = "전환 조건", BaseStruct = "/Script/MNYS.YSPlaybackCondition", ExcludeBaseStruct))
	TArray<FInstancedStruct> TransitionConditions;

	/** 같은 출력에서 나가는 화살표가 여럿일 때의 우선순위. 작을수록 먼저 평가된다. */
	UPROPERTY(EditAnywhere, Category = "YS | Transition", meta = (DisplayName = "우선순위 (작을수록 먼저)"))
	int32 PriorityOrder = 0;

	/** 이 화살표가 확정되면 전환 직전에 실행한다. 목적지가 유지·종료여도 돈다. */
	UPROPERTY(EditAnywhere, Instanced, Category = "YS | Transition", meta = (DisplayName = "경로 행동"))
	TArray<TObjectPtr<UYSPlaybackAction>> Actions;

	/** 출발 상태에서 이 화살표가 가리키는 Task. 인덱스가 어긋났으면 nullptr. */
	UYSPlaybackTask* GetSourceTask() const;

	/** Task 와 출력이 둘 다 실재한다. 아니면 컴파일에서 빠지고 그래프에서 붉게 뜬다. */
	bool HasValidOutput() const;

	/** From 의 출력 → 이 노드 → To 의 입력으로 잇는다. */
	void CreateConnections(UYSPlaybackGraphNode_Base* From, UYSPlaybackGraphNode_Base* To);

	/** 이 전환이 향하는 노드. 상태일 수도, 종료·유지일 수도, 아무것도 아닐 수도 있다. */
	UYSPlaybackGraphNode_Base* GetTargetNode() const;

	/** 목적지가 아예 없다. 의도인지 실수인지 알 수 없으므로 그래프에서 붉게 표시한다. */
	bool IsDangling() const { return GetTargetNode() == nullptr; }

	/** 이 전환이 향하는 상태. 종료·유지로 가거나 연결이 없으면 nullptr. */
	UYSPlaybackGraphNode_State* GetNextState() const;

	/** 이 전환이 나온 상태. 전환 위젯이 두 상태 사이에 자리를 잡을 때 쓴다. */
	UYSPlaybackGraphNode_State* GetPreviousState() const;

	//~ UEdGraphNode
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FText GetTooltipText() const override;
	//~ End
};
