// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

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

	/** 스킬 입력은 기술이 아니라 "슬롯"을 가리킨다. 기술 이름으로 태그를 파면 6인 × 기술 수 × State 12종으로 조합이 불어나서, 6인이 슬롯 태그를 공유하고 어느 기술을 넣을지는 FYSGrantedAbilityData::InputTag 가 정한다. */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputSkill1);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputSkill2);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputUltimate);

	// 세이브(혼). 다른 캐릭터는 이 태그에 그랜트된 어빌리티가 없어 무반응이다.
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputSave);

	// 폼 전환(마보마이 V). Form.Berserker/Form.Guardian 부여 어빌리티가 이 슬롯을 쓴다.
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputForm);

	// 전투 타겟 이전/다음. 어빌리티가 아니라 UYSCombatTargetComponent 가 원본 입력으로 받는다.
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTargetPrev);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTargetNext);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputUpAttack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputDownAttack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputBackDodgeAttack);
}
