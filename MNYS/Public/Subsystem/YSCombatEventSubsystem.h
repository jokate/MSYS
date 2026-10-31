// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "YSCombatEventSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FYSOnManualTargetSwitched, AActor* /*NewTarget*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FYSOnCombatPhaseChanged, AActor* /*PhaseOwner*/, int32 /*NewPhase*/);

/** 전투 알림 허브. 컴포넌트 · 서브시스템끼리 서로를 찾아 부르지 않고 여기 델리게이트로만 알린다. */
UCLASS()
class MNYS_API UYSCombatEventSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public :
	static UYSCombatEventSubsystem* Get(const UObject* WorldContextObject);

	// 플레이어가 Q/E 로 타겟을 바꿨을 때. 첫 잡기도 포함한다.
	FYSOnManualTargetSwitched OnManualTargetSwitched;

	// 페이즈가 오른 직후. 듣는 쪽이 PhaseOwner 로 자기 액터인지 가린다.
	FYSOnCombatPhaseChanged OnPhaseChanged;
};
