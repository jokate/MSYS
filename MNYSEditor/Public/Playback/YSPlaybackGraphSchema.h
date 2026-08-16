// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphSchema.h"
#include "YSPlaybackGraphSchema.generated.h"

/**
 * 플레이백 그래프의 규칙.
 *
 * 핵심은 하나다 — 상태끼리 이으면 전환 노드가 자동으로 태어난다.
 * 사용자는 선을 그었다고 느끼지만 실제로는 노드가 하나 생기고 연결이 둘 생긴다.
 * 엔진의 UAnimationStateMachineSchema 가 쓰는 방식과 같다
 * (CONNECT_RESPONSE_MAKE_WITH_CONVERSION_NODE → CreateAutomaticConversionNodeAndConnections).
 */
UCLASS()
class UYSPlaybackGraphSchema : public UEdGraphSchema
{
	GENERATED_BODY()

public:
	/** 이 그래프의 유일한 핀 타입. 데이터가 아니라 순서만 흐른다. */
	static const FName PC_Transition;

	//~ UEdGraphSchema
	virtual void CreateDefaultNodesForGraph(UEdGraph& Graph) const override;
	virtual void GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const override;
	virtual const FPinConnectionResponse CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const override;
	virtual bool CreateAutomaticConversionNodeAndConnections(UEdGraphPin* A, UEdGraphPin* B) const override;
	virtual FLinearColor GetPinTypeColor(const FEdGraphPinType& PinType) const override;
	virtual void BreakPinLinks(UEdGraphPin& TargetPin, bool bSendsNodeNotifcation) const override;
	//~ End
};
