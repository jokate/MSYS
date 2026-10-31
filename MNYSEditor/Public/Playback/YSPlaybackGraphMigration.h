// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class UYSPlaybackGraphAsset;
class UYSAbilityPlaybackBase;

// 원본은 건드리지 않는다 — 그래프가 마음에 안 들면 에셋만 버리면 되고 어빌리티는 예전대로 돈다.
class FYSPlaybackGraphMigration
{
public:
	// 기존 그래프 내용은 전부 지워진다. 반환값은 옮긴 상태 노드 개수(0 이면 아무것도 안 옮겨짐).
	static int32 ImportFromSourceAbility(UYSPlaybackGraphAsset* Asset);

	// 플레이백 배열(태스크 경로 포함)을 상태 · 전환 노드로 편다. StateNames 는 노드 제목, 비우면 클래스 이름.
	static int32 ImportPlaybacks(UYSPlaybackGraphAsset* Asset, const TArray<TObjectPtr<UYSAbilityPlaybackBase>>& SourcePlaybacks, const TArray<FString>& StateNames = {});
};
