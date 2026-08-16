// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class UYSPlaybackGraphAsset;

/**
 * 편집용 그래프를 런타임 데이터로 굽는다.
 *
 * 왜 복제하는가 —
 * 편집용 노드가 소유한 플레이백 오브젝트는 EdGraph 아래에 매달려 있고,
 * EdGraph 는 WITH_EDITORONLY_DATA 라 쿠킹 때 통째로 사라진다.
 * 참조만 옮기면 패키징한 빌드에서 전부 null 이 된다.
 *
 * 언제 부르는가 —
 * 구조가 바뀔 때마다 즉시. USoundCue 가 연결·해제·노드 추가마다
 * CompileSoundNodesFromGraphNodes 를 부르는 것과 같은 정책이다.
 * (Editor/AudioEditor/Private/SoundCueGraphSchema.cpp:385,405,417)
 * 노드가 어빌리티당 한 자릿수라 매번 다시 굽는 비용이 무시할 수준이다.
 */
class FYSPlaybackGraphCompiler
{
public:
	/**
	 * 그래프를 읽어 에셋의 Playbacks 배열을 다시 만든다.
	 *
	 * 정렬 규칙 — 시작 노드가 가리키는 상태가 항상 0번이다.
	 * 어빌리티의 SetupPlayBack 이 0번부터 시작하므로 이 규칙이 곧 진입점이다.
	 */
	static void Compile(UYSPlaybackGraphAsset* Asset);
};
