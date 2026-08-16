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

	//~ UEdGraphNode
	virtual void AllocateDefaultPins() override;
	virtual void PostPlacedNewNode() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FText GetTooltipText() const override;
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
 * 전환 = 노드.
 *
 * 화면에서는 화살표로 보이지만 실제로는 상태 A 와 상태 B 사이에 낀 노드다.
 * 이렇게 두는 이유는 하나다 — 그래프 에디터의 선택 단위가 노드라서,
 * 전환을 노드로 만들어야 클릭 한 번에 그 전환의 조건만 Details 에 띄울 수 있다.
 *
 * 엔진의 UAnimStateTransitionNode 가 같은 이유로 같은 선택을 했다.
 */
UCLASS()
class UYSPlaybackGraphNode_Transition : public UYSPlaybackGraphNode_Base
{
	GENERATED_BODY()

public:
	/**
	 * 전환 규칙.
	 *
	 * NextNodeIndex 는 여기서 의미가 없다 — 연결이 곧 다음 노드다.
	 * 컴파일 때 연결을 보고 채워 넣는다. Details 에 뜨더라도 건드리지 마라.
	 */
	UPROPERTY(EditAnywhere, Category = "YS | Transition", meta = (DisplayName = "전환 규칙"))
	FYSPlaybackEdge Edge;

	/**
	 * 같은 노드에서 나가는 전환이 여럿일 때의 우선순위. 작을수록 먼저 평가된다.
	 * 지금의 Transitions 배열 순서를 대신한다.
	 */
	UPROPERTY(EditAnywhere, Category = "YS | Transition", meta = (DisplayName = "우선순위 (작을수록 먼저)"))
	int32 PriorityOrder = 0;

	/** From 의 출력 → 이 노드 → To 의 입력으로 잇는다. */
	void CreateConnections(UYSPlaybackGraphNode_Base* From, UYSPlaybackGraphNode_Base* To);

	/** 이 전환이 향하는 상태. 연결이 없으면 nullptr — 체인 종료를 뜻한다. */
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
