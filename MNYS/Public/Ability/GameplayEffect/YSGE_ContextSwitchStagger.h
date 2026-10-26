// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "YSGE_ContextSwitchStagger.generated.h"

/**
 * 컨텍스트 스위치 변형이 등급 0 · 1 적에게 거는 경직 (그로기 피해 대신).
 * 시간은 UYSPlaybackTask_ContextSwitchHit 이 행의 경직 길이 × 남용 감쇠로 SetByCaller 에 넣고, AI 억제 태그는 스펙에 붙인다.
 */
UCLASS()
class MNYS_API UYSGE_ContextSwitchStagger : public UGameplayEffect
{
	GENERATED_BODY()

public :
	UYSGE_ContextSwitchStagger();

	static const FName DurationName;
};
