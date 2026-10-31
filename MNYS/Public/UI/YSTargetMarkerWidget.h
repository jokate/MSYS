// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "YSTargetMarkerWidget.generated.h"

/** 현재 타겟 표지: 적 몸통 좌우 꺾쇠 [ ]. 화면 전체를 덮는 캔버스 안에서 LeftBracket · RightBracket 을 대상의 화면 영역 옆에 세운다. 대상은 이벤트로만 바뀌고 틱은 화면 투영만 한다. */
UCLASS(Abstract)
class MNYS_API UYSTargetMarkerWidget : public UUserWidget
{
	GENERATED_BODY()

public :
	/** 같은 프레임에 표지를 옮긴다. nullptr 이면 직전 대상에 흐린 표지를 잠시 남긴다. */
	void SetTarget(AActor* NewTarget);

protected :
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private :
	bool GetScreenRect(const AActor* Actor, FVector2D& OutMin, FVector2D& OutMax) const;
	void PlaceBrackets(const FVector2D& ScreenMin, const FVector2D& ScreenMax);
	void HideMarker();
	void SetBracketsVisible(bool bVisible);

private :
	// 둘 다 캔버스 패널 자식이어야 한다. 슬롯 폭이 꺾쇠 폭이고 높이는 대상에 맞춘다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> LeftBracket;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> RightBracket;

	UPROPERTY(EditAnywhere, Category = "YS | HUD", meta = (DisplayName = "몸통과 꺾쇠 사이 (px)"))
	float BracketGap = 4.f;

	UPROPERTY(EditAnywhere, Category = "YS | HUD", meta = (DisplayName = "이탈 후 표지 유지 (초)", ClampMin = "0"))
	float LostFadeDuration = 1.f;

	UPROPERTY(EditAnywhere, Category = "YS | HUD", meta = (DisplayName = "이탈 후 불투명도", ClampMin = "0", ClampMax = "1"))
	float LostOpacity = 0.5f;

	TWeakObjectPtr<AActor> Target;

	// 대상을 잃은 시각. 음수면 지금 대상이 있다.
	float LostTime = -1.f;
};
