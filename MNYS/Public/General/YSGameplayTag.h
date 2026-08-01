// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
/**
 * 
 */

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
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(JustAvoid_Window);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Invincible);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_OnDead);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_DeathComplete);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_JustAvoid);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Attack_Activate);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_TagEnter);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Buff_JustAvoid);

	// 입력 위상 — 플레이백 컨텍스트 태그로만 쓰인다.
	// 어빌리티 라우팅은 EYSInputPhase 파라미터가 담당하고, 이 두 태그는
	// FYSPlaybackEdge의 전환 조건이 "뗄 때 전환"을 표현할 수 있게 하는 용도다.
	// (차지 → 릴리즈 발사 같은 노드 전환을 태그 조합 폭발 없이 처리한다)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Phase_Pressed);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Phase_Released);

	// 스탯 초기화 SetByCaller — GE_StatInit 의 Set By Caller 모디파이어와 1:1 대응한다.
	// 문자열 조회(RequestGameplayTag) 대신 네이티브 태그를 쓰는 이유는, 미등록/오타 시
	// 무효 태그가 반환되어 마그니튜드가 조용히 0이 되는 사고를 컴파일 타임에 막기 위함이다.
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Stat_HP);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Stat_MEL);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Stat_RNG);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Stat_AGI);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Stat_SYN);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Stat_SCL);

	// AI 억제 — 캐릭터 고유 (ASC에 부여, ActivationOwnedTags/루즈 태그)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(AISuppress);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(AISuppress_HitReact);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(AISuppress_Skill);

	// AI 억제 — 월드 전역 (UYSWorldTagSubsystem에 부여)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(World_AISuppress);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(World_AISuppress_Cinematic);
};