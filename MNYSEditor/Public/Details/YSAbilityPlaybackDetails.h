// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

/**
 * 그래프 안에서 보는 플레이백에서 Transitions 배열을 숨긴다.
 *
 * 그래프에서는 전환 노드가 그 배열을 만들고 컴파일이 통째로 덮어쓴다.
 * 그대로 두면 "고쳐도 사라지는 칸"이 남는다.
 *
 * 레거시 어빌리티의 인라인 배열에서는 그대로 보여야 한다 —
 * 아직 마이그레이션하지 않은 어빌리티는 이 배열이 유일한 편집 수단이기 때문이다.
 * 그래서 클래스가 아니라 Outer 를 보고 판단한다.
 */
class FYSAbilityPlaybackDetails : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};
