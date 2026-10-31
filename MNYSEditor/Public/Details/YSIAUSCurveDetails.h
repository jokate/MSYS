// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class SResponseCurvePreviewWidget;

// 엔진이 EditInlineNew 중첩 객체에는 클래스 커스터마이제이션을 실행하지 않으므로 바깥 어빌리티 클래스에 건다.
class MNYSEDITOR_API FYSIAUSCurveDetails : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	TArray<TSharedPtr<SResponseCurvePreviewWidget>> PreviewWidgets;
};
