// Fill out your copyright notice in the Description page of Project Settings.


#include "Playback/YSPlaybackGraphNodeFactory.h"

#include "Playback/SGraphNodeYSPlaybackTransition.h"
#include "Playback/YSPlaybackGraphNode.h"

TSharedPtr<SGraphNode> FYSPlaybackGraphNodeFactory::CreateNode(UEdGraphNode* Node) const
{
	// 전환만 특별 취급한다. 상태·시작 노드는 기본 외형으로 충분하다.
	if (UYSPlaybackGraphNode_Transition* TransitionNode = Cast<UYSPlaybackGraphNode_Transition>(Node))
	{
		return SNew(SGraphNodeYSPlaybackTransition, TransitionNode);
	}

	return nullptr;
}
