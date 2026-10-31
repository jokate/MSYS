// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "YSGE_GroggyBreak.generated.h"

/** 브레이크 상태. 시간 · 피해 배율 · 부여 태그는 UYSGroggyComponent 가 FYSGroggyRules 에서 SetByCaller 와 동적 태그로 넣는다 (애셋 불필요). Cue · 추가 태그를 쓰려면 BP 로 파생해 설정에 지정한다. */
UCLASS()
class MNYS_API UYSGE_GroggyBreak : public UGameplayEffect
{
	GENERATED_BODY()

public :
	UYSGE_GroggyBreak();
};
