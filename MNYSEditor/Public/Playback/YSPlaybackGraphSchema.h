// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphSchema.h"
#include "YSPlaybackGraphSchema.generated.h"

// 상태끼리 이으면 전환 노드가 자동으로 생긴다 — 노드 하나에 연결 둘.
// (CONNECT_RESPONSE_MAKE_WITH_CONVERSION_NODE → CreateAutomaticConversionNodeAndConnections)
UCLASS()
class UYSPlaybackGraphSchema : public UEdGraphSchema
{
	GENERATED_BODY()

public:
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
