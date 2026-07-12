// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class SResponseCurvePreviewWidget;

/**
 * UYSGameplayAbility_AIBase 디테일 패널에 UtilityScore 배열 요소별 응답 커브 프리뷰를 추가한다.
 * 엔진이 EditInlineNew 중첩 객체에는 클래스 커스터마이제이션을 실행하지 않으므로(DetailLayoutHelpers.cpp
 * "Edit inline new children are not supported for customization yet") 바깥 어빌리티 클래스에 건다.
 */
class MNYSEDITOR_API FYSIAUSCurveDetails : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	TArray<TSharedPtr<SResponseCurvePreviewWidget>> PreviewWidgets;
};
