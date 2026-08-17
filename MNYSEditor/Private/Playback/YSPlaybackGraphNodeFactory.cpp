// Fill out your copyright notice in the Description page of Project Settings.


#include "Playback/YSPlaybackGraphNodeFactory.h"

#include "Playback/SGraphNodeYSPlaybackState.h"
#include "Playback/SGraphNodeYSPlaybackTransition.h"
#include "Playback/YSPlaybackGraphNode.h"

TSharedPtr<SGraphNode> FYSPlaybackGraphNodeFactory::CreateNode(UEdGraphNode* Node) const
{
	if (UYSPlaybackGraphNode_Transition* TransitionNode = Cast<UYSPlaybackGraphNode_Transition>(Node))
	{
		return SNew(SGraphNodeYSPlaybackTransition, TransitionNode);
	}

	if (UYSPlaybackGraphNode_Entry* EntryNode = Cast<UYSPlaybackGraphNode_Entry>(Node))
	{
		return SNew(SGraphNodeYSPlaybackEntry, EntryNode);
	}

	if (UYSPlaybackGraphNode_Exit* ExitNode = Cast<UYSPlaybackGraphNode_Exit>(Node))
	{
		return SNew(SGraphNodeYSPlaybackExit, ExitNode);
	}

	if (UYSPlaybackGraphNode_Stay* StayNode = Cast<UYSPlaybackGraphNode_Stay>(Node))
	{
		return SNew(SGraphNodeYSPlaybackStay, StayNode);
	}

	if (UYSPlaybackGraphNode_State* StateNode = Cast<UYSPlaybackGraphNode_State>(Node))
	{
		return SNew(SGraphNodeYSPlaybackState, StateNode);
	}

	return nullptr;
}
