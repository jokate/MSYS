// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Subsystem/YSSquadSubsystem.h"
#include "YSSwitchAttackToastWidget.generated.h"

class UTextBlock;

/** 스위치 공격 표시. 변형이 나가면 행 이름 · 임팩트 등급을 잠깐 띄운다. 보고는 UYSCombatHUDComponent 가 스쿼드의 OnSwitchAttackFired 에서 받아 내리고, 모양은 WBP 가 OnToastShown / OnToastHidden 에서 정한다. */
UCLASS(Abstract)
class MNYS_API UYSSwitchAttackToastWidget : public UUserWidget
{
	GENERATED_BODY()

public :
	/** 이미 떠 있으면 새 보고로 바꾸고 시간을 처음부터 다시 잰다. */
	void ShowReport(const FYSSwitchAttackReport& Report);

protected :
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "YS | HUD")
	void OnToastShown(const FYSSwitchAttackReport& Report);

	UFUNCTION(BlueprintImplementableEvent, Category = "YS | HUD")
	void OnToastHidden();

private :
	void HideToast();

private :
	UPROPERTY(EditAnywhere, Category = "YS | HUD", meta = (DisplayName = "표시 시간 (초)", ClampMin = "0"))
	float DisplayDuration = 0.6f;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RowText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> GradeText;

	// 감쇠 배율. 감쇠가 없으면 숨긴다.
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ScaleText;

	// 첫 잡기로 연 창일 때만 보이는 표지
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> FirstGrabMark;

	FTimerHandle HideTimer;
};
