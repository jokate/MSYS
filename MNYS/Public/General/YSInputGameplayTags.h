// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

/**
 * 
 */
namespace YSInputTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputMove);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputLook);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputAttack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputDodge);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputJump);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputSprint);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputLeft);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputRight);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputUp);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputDown);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputAim);

	/**
	 * 스킬 입력은 기술이 아니라 "슬롯"을 가리킨다.
	 *
	 * 기술 이름으로 태그를 파면(Input.Iai 같은) 6인 × 기술 수 × State 12종만큼
	 * 조합이 불어난다. 슬롯으로 두면 6인이 같은 태그를 공유하고, 누가 어떤
	 * 기술을 그 슬롯에 넣을지는 FYSGrantedAbilityData::InputTag 가 정한다.
	 */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputSkill1);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputSkill2);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputUltimate);

	// 세이브(혼). 다른 캐릭터는 이 태그에 그랜트된 어빌리티가 없어 무반응이다.
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputSave);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputUpAttack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputDownAttack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputBackDodgeAttack);
}
