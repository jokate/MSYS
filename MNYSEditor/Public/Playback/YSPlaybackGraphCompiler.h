// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class UYSPlaybackGraphAsset;
class UYSPlaybackGraphNode_State;
class UYSPlaybackGraphNode_Transition;

// 복제하는 이유 — EdGraph 는 WITH_EDITORONLY_DATA 라 쿠킹 때 사라지므로, 참조만 옮기면 패키징 빌드에서 전부 null 이 된다.
// 구조가 바뀔 때마다 부른다. 노드가 어빌리티당 한 자릿수라 매번 굽는 비용은 무시할 수준이다.
class FYSPlaybackGraphCompiler
{
public:
	// 시작 노드가 가리키는 상태가 항상 0번이다 — 어빌리티의 SetupPlayBack 이 0번부터 시작한다.
	static void Compile(UYSPlaybackGraphAsset* Asset);

	// 우선순위 오름차순이며, 같으면 그래프 등록 순서를 따른다.
	static void GatherOutgoingTransitions(const UYSPlaybackGraphNode_State& State, TArray<UYSPlaybackGraphNode_Transition*>& OutTransitions);
};
