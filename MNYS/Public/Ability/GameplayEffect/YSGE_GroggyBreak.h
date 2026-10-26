// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "YSGE_GroggyBreak.generated.h"

/**
 * 브레이크 상태. 시간과 받는 피해 배율은 UYSGroggyComponent 가 FYSGroggyRules 에서 SetByCaller 로 넣고,
 * 부여 태그(State.Groggy.Broken 외)도 스펙에 동적으로 붙인다 — 애셋 없이 이 클래스만으로 돈다.
 * Cue · 추가 태그를 애셋 쪽에서 더하려면 이 클래스를 BP 로 파생해 설정에 지정한다.
 */
UCLASS()
class MNYS_API UYSGE_GroggyBreak : public UGameplayEffect
{
	GENERATED_BODY()

public :
	UYSGE_GroggyBreak();

	static const FName DurationName;
	static const FName DamageTakenMultiplierName;
};
