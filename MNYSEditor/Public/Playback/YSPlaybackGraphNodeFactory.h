// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EdGraphUtilities.h"

/**
 * 플레이백 그래프 노드의 외형을 꽂는 자리.
 *
 * 엔진은 노드 클래스와 위젯을 직접 묶지 않는다 — 등록된 팩토리들에게 차례로 물어본다.
 * 그래서 커스텀 외형은 모듈 시작 시 FEdGraphUtilities::RegisterVisualNodeFactory 로 등록한다.
 * (AnimationBlueprintEditorModule.cpp:56 이 같은 자리)
 */
class FYSPlaybackGraphNodeFactory : public FGraphPanelNodeFactory
{
public:
	virtual TSharedPtr<class SGraphNode> CreateNode(UEdGraphNode* Node) const override;
};
