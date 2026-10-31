// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Styling/SlateBrush.h"
#include "YSNearbyRadarWidget.generated.h"

class UCanvasPanel;
class UImage;

/** 주변 적 레이더. 원 둘레에 감지된 적마다 점을 세운다 — 위가 카메라 정면이다. 적 목록 · 현재 타겟은 UYSCombatHUDComponent 가 이벤트로 내리고 틱은 점 위치만 옮긴다. 점은 PipCanvas 아래에 코드가 만들며 모양은 브러시 두 개(일반 · 현재 타겟)로만 정한다. */
UCLASS(Abstract)
class MNYS_API UYSNearbyRadarWidget : public UUserWidget
{
	GENERATED_BODY()

public :
	UYSNearbyRadarWidget(const FObjectInitializer& ObjectInitializer);

	void SetEnemies(const TArray<AActor*>& NewEnemies);
	void SetCurrentTarget(AActor* NewTarget);

protected :
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private :
	void ApplyPipStyles();
	UImage* GetOrCreatePip(int32 Index);

private :
	// 원 중심이 이 캔버스의 중심이다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> PipCanvas;

	UPROPERTY(EditAnywhere, Category = "YS | HUD", meta = (DisplayName = "원 반지름 (px)", ClampMin = "0"))
	float RadarRadius = 90.f;

	// 위쪽을 바깥으로 그린 모양이면 점이 원 밖을 가리킨다.
	UPROPERTY(EditAnywhere, Category = "YS | HUD", meta = (DisplayName = "적 점 브러시"))
	FSlateBrush PipBrush;

	UPROPERTY(EditAnywhere, Category = "YS | HUD", meta = (DisplayName = "현재 타겟 점 브러시"))
	FSlateBrush TargetPipBrush;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UImage>> Pips;

	TArray<TWeakObjectPtr<AActor>> Enemies;

	TWeakObjectPtr<AActor> CurrentTarget;
};
