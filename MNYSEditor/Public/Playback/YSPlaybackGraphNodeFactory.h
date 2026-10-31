// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EdGraphUtilities.h"

// 엔진은 노드 클래스와 위젯을 직접 묶지 않고 등록된 팩토리들에게 차례로 물어본다.
class FYSPlaybackGraphNodeFactory : public FGraphPanelNodeFactory
{
public:
	virtual TSharedPtr<class SGraphNode> CreateNode(UEdGraphNode* Node) const override;
};
