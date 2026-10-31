// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraph.h"
#include "YSPlaybackGraph.generated.h"

class UYSPlaybackGraphAsset;

// 실질 데이터가 대부분 WITH_EDITORONLY_DATA 라 에디터 모듈에 둔다 — 런타임이 알 필요가 없다.
UCLASS()
class UYSPlaybackGraph : public UEdGraph
{
	GENERATED_BODY()

public:
	UYSPlaybackGraphAsset* GetPlaybackAsset() const;
};
