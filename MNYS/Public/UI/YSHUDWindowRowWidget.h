// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "Blueprint/UserWidget.h"
#include "YSHUDWindowRowWidget.generated.h"

class UGameplayEffect;
class UProgressBar;
class UTextBlock;
struct FGameplayEffectSpec;

/** 시간창 한 종류의 표시 규칙. 새 창(저스트 등)은 이 행을 하나 더한다. */
USTRUCT(BlueprintType)
struct FYSHUDWindowEntry
{
	GENERATED_BODY()

	// 이 GE 가 걸려 있는 동안을 창으로 본다. 지속 시간이 곧 창 길이다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | HUD", meta = (DisplayName = "창 GE"))
	TSubclassOf<UGameplayEffect> Effect;

	// 여러 창이 함께 열리면 높은 쪽 하나만 보인다 (기획 §4).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | HUD", meta = (DisplayName = "우선순위"))
	int32 Priority = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | HUD", meta = (DisplayName = "문구"))
	FText Label;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | HUD", meta = (DisplayName = "색"))
	FLinearColor Color = FLinearColor::White;
};

/**
 * ③ 스위치 창 · ④ 남용 쿨 (기획 11_UIUX_V4_DRAFT §3.3). 하단 아이콘 줄 하나에 시간창을 모은다.
 * 열림 · 닫힘은 UYSCombatHUDComponent 가 활성 폰 ASC 의 GE 추가 · 제거로 알려 준다.
 * 아이콘 교체 같은 모양은 OnWindowShown / OnWindowHidden 에서 WBP 가 한다.
 */
UCLASS(Abstract)
class MNYS_API UYSHUDWindowRowWidget : public UUserWidget
{
	GENERATED_BODY()

public :
	void HandleEffectAdded(const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle Handle);
	void HandleEffectRemoved(FActiveGameplayEffectHandle Handle);

	/** 폰 교대로 구독하던 ASC 가 바뀔 때 부른다. 남용 쿨은 스쿼드 기록이라 남긴다. */
	void ClearWindows();

	/** 남은 시간부터 막대를 다시 채운다. 끝나는 시각이 지금 막대와 같으면 그대로 두고, 0 이하면 숨긴다. */
	void StartAbuseCooldown(float Seconds);

protected :
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "YS | HUD")
	void OnWindowShown(const FYSHUDWindowEntry& Entry);

	UFUNCTION(BlueprintImplementableEvent, Category = "YS | HUD")
	void OnWindowHidden();

private :
	struct FOpenWindow
	{
		int32 EntryIndex = INDEX_NONE;
		FActiveGameplayEffectHandle Handle;
		float StartTime = 0.f;
		float Duration = 0.f;
	};

	void RefreshShownWindow();
	float GetNow() const;

private :
	UPROPERTY(EditAnywhere, Category = "YS | HUD", meta = (DisplayName = "시간창 표"))
	TArray<FYSHUDWindowEntry> Entries;

	// 창이 열린 동안만 보이는 묶음
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> WindowPanel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> WindowLabel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> WindowTimeBar;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> AbuseCooldownBar;

	TArray<FOpenWindow> OpenWindows;

	int32 ShownEntryIndex = INDEX_NONE;

	float AbuseCooldownEndTime = -1.f;
	float AbuseCooldownDuration = 0.f;
};
