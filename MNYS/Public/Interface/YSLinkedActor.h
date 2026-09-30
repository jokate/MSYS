// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "General/YSEnum.h"
#include "YSLinkedActor.generated.h"

UINTERFACE()
class UYSLinkedActor : public UInterface
{
	GENERATED_BODY()
};

/**
 * 스킬 사용 1회(HitContext)에 연결되어 해제 명령을 받는 액터.
 * 구현한 액터만 SpawnByConfig 에서 등록된다 — 쏘고 잊는 투사체는 구현하지 않는다. 취소 시 같이 걷히기 때문.
 */
class MNYS_API IYSLinkedActor
{
	GENERATED_BODY()

public:
	virtual void Release(EYSReleaseReason Reason) = 0;
};
