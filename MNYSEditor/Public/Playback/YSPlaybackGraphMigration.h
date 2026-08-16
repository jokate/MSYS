// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class UYSPlaybackGraphAsset;

/**
 * 기존 어빌리티의 레거시 Playbacks 배열을 그래프로 옮긴다.
 *
 * 원본은 건드리지 않는다 — 어빌리티의 배열은 그대로 남는다.
 * 그래프가 마음에 안 들면 에셋만 버리면 되고, 어빌리티는 예전대로 돈다.
 */
class FYSPlaybackGraphMigration
{
public:
	/**
	 * 에셋의 SourceAbility 를 읽어 그래프를 다시 만든다.
	 * 기존 그래프 내용은 전부 지워진다.
	 *
	 * @return 옮긴 상태 노드 개수. 0 이면 아무것도 안 옮겨졌다.
	 */
	static int32 ImportFromSourceAbility(UYSPlaybackGraphAsset* Asset);
};
