// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraph.h"
#include "YSPlaybackGraph.generated.h"

class UYSPlaybackGraphAsset;

/**
 * 플레이백 편집용 그래프.
 *
 * UEdGraph 자체는 Engine 모듈에 있지만 실질 데이터는 대부분 WITH_EDITORONLY_DATA 다.
 * 그래서 이 클래스는 에디터 모듈에 둔다 — 런타임이 이걸 알 필요가 없다.
 */
UCLASS()
class UYSPlaybackGraph : public UEdGraph
{
	GENERATED_BODY()

public:
	/** 이 그래프를 소유한 에셋. Outer 가 곧 에셋이다. */
	UYSPlaybackGraphAsset* GetPlaybackAsset() const;
};
