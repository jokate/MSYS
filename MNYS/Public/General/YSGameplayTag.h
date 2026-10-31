// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

namespace YSTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(BlockInput);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(BlockLockOn);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(AcceptAbilityInput);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_TraceStart);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_TraceEnd);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_OnHit);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_ApplyVelocity);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_SpawnActor);
	
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Hit_Normal);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Hit_Big);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Hit_Medium);
	
	// 4방향 태그. 공격자가 실어보내는 값이 아니라, 피격/이동 시점에 로컬에서 계산해 조회 태그로 합성한다.
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Direction_Forward);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Direction_Backward);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Direction_Left);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Direction_Right);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(JustAvoid_Window);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Invincible);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_OnDead);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_OnKill);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_DeathComplete);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_JustAvoid);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Attack_Activate);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_TagEnter);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_PushCamera);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_PopCamera);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_AimStart);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_AimStop);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_ConsumeResource);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Record);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Replay);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_DeployEcho);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_OnLand);
	// 그로기 게이지(Groggy)가 0 에 닿은 순간. 브레이크 연출은 이 이벤트를 받는 쪽이 맡는다.
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_GroggyBroken);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Buff_JustAvoid);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Passive_RelativeEvaluation);

	// 입력 위상 — 플레이백 컨텍스트 태그로만 쓰인다. 플레이백 경로의 전환 조건이 "뗄 때 전환"(차지 → 릴리즈 발사)을 태그 조합 폭발 없이 표현하게 하는 용도다.
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Phase_Pressed);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Phase_Released);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Phase_Held);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Stat_HP);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Stat_MEL);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Stat_RNG);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Stat_AGI);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Stat_SYN);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Stat_SCL);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Stat_RangeAttackCount);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Groggy_Damage);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Groggy_SwitchEntryDamage);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Groggy_BreakDuration);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Groggy_DamageTakenMultiplier);
	
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Resource_Empty);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Resource_NotFull);

	// 세이브(혼). 슬롯 개수를 태그로 세지 않는 이유는 YSSaveComponent 주석 참고.
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Save_Pending);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Save_HasSlot);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Save_Full);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Echo);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Save_TripleEcho);

	// 브레이크 중인 적. UYSGE_GroggyBreak 가 부여한다 (UI · 상태별 피해 배율 · 페이즈 전환 보류).
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Groggy_Broken);

	// AI 억제 — 캐릭터 고유 (ASC에 부여, ActivationOwnedTags/루즈 태그)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(AISuppress);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(AISuppress_HitReact);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(AISuppress_Skill);

	// AI 억제 — 월드 전역 (UYSWorldTagSubsystem에 부여)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(World_AISuppress);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(World_AISuppress_Cinematic);
};