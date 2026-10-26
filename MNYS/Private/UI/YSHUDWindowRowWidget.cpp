// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/YSHUDWindowRowWidget.h"

#include "GameplayEffect.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"

void UYSHUDWindowRowWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if ( IsValid(WindowPanel) )
	{
		WindowPanel->SetVisibility(ESlateVisibility::Collapsed);
	}

	if ( IsValid(AbuseCooldownBar) )
	{
		AbuseCooldownBar->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UYSHUDWindowRowWidget::HandleEffectAdded(const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle Handle)
{
	if ( Spec.Def == nullptr )
	{
		return;
	}

	const int32 EntryIndex = Entries.IndexOfByPredicate([&Spec](const FYSHUDWindowEntry& Entry)
	{
		return Entry.Effect == Spec.Def->GetClass();
	});

	if ( EntryIndex == INDEX_NONE )
	{
		return;
	}

	FOpenWindow& Window = OpenWindows.AddDefaulted_GetRef();
	Window.EntryIndex = EntryIndex;
	Window.Handle = Handle;
	Window.StartTime = GetNow();
	Window.Duration = Spec.GetDuration();

	RefreshShownWindow();
}

void UYSHUDWindowRowWidget::HandleEffectRemoved(FActiveGameplayEffectHandle Handle)
{
	if ( OpenWindows.RemoveAll([Handle](const FOpenWindow& Window) { return Window.Handle == Handle; }) > 0 )
	{
		RefreshShownWindow();
	}
}

void UYSHUDWindowRowWidget::ClearWindows()
{
	OpenWindows.Reset();
	RefreshShownWindow();
}

void UYSHUDWindowRowWidget::StartAbuseCooldown(float Seconds)
{
	if ( Seconds <= 0.f )
	{
		AbuseCooldownEndTime = -1.f;

		if ( IsValid(AbuseCooldownBar) )
		{
			AbuseCooldownBar->SetVisibility(ESlateVisibility::Collapsed);
		}

		return;
	}

	const float EndTime = GetNow() + Seconds;

	// 끝나는 시각이 같으면 같은 쿨이 이어지는 것이다 (전역 쿨 중 타겟 변경). 다시 채우지 않고 비율을 유지한다.
	if ( AbuseCooldownEndTime >= 0.f && FMath::IsNearlyEqual(EndTime, AbuseCooldownEndTime, 0.01f) )
	{
		return;
	}

	AbuseCooldownDuration = Seconds;
	AbuseCooldownEndTime = EndTime;

	if ( IsValid(AbuseCooldownBar) )
	{
		AbuseCooldownBar->SetPercent(1.f);
		AbuseCooldownBar->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void UYSHUDWindowRowWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	const float Now = GetNow();

	// 제거 알림을 놓쳐도 창이 남지 않게 지속 시간으로도 닫는다. 지속 시간이 없는(무한) GE 는 제거 알림만 믿는다.
	const int32 Expired = OpenWindows.RemoveAll([Now](const FOpenWindow& Window)
	{
		return Window.Duration > 0.f && Now - Window.StartTime >= Window.Duration;
	});

	if ( Expired > 0 )
	{
		RefreshShownWindow();
	}

	if ( ShownEntryIndex != INDEX_NONE && IsValid(WindowTimeBar) )
	{
		const FOpenWindow* Shown = OpenWindows.FindByPredicate([this](const FOpenWindow& Window) { return Window.EntryIndex == ShownEntryIndex; });

		if ( Shown != nullptr && Shown->Duration > 0.f )
		{
			WindowTimeBar->SetPercent(FMath::Clamp(1.f - (Now - Shown->StartTime) / Shown->Duration, 0.f, 1.f));
		}
	}

	if ( AbuseCooldownEndTime >= 0.f && IsValid(AbuseCooldownBar) )
	{
		const float Remaining = AbuseCooldownEndTime - Now;

		if ( Remaining <= 0.f )
		{
			AbuseCooldownEndTime = -1.f;
			AbuseCooldownBar->SetVisibility(ESlateVisibility::Collapsed);
		}
		else
		{
			AbuseCooldownBar->SetPercent(Remaining / AbuseCooldownDuration);
		}
	}
}

void UYSHUDWindowRowWidget::RefreshShownWindow()
{
	int32 BestEntry = INDEX_NONE;

	for ( const FOpenWindow& Window : OpenWindows )
	{
		if ( BestEntry == INDEX_NONE || Entries[Window.EntryIndex].Priority > Entries[BestEntry].Priority )
		{
			BestEntry = Window.EntryIndex;
		}
	}

	if ( BestEntry == ShownEntryIndex )
	{
		return;
	}

	ShownEntryIndex = BestEntry;

	if ( ShownEntryIndex == INDEX_NONE )
	{
		if ( IsValid(WindowPanel) )
		{
			WindowPanel->SetVisibility(ESlateVisibility::Collapsed);
		}

		OnWindowHidden();
		return;
	}

	const FYSHUDWindowEntry& Entry = Entries[ShownEntryIndex];

	if ( IsValid(WindowLabel) )
	{
		WindowLabel->SetText(Entry.Label);
		WindowLabel->SetColorAndOpacity(Entry.Color);
	}

	if ( IsValid(WindowTimeBar) )
	{
		WindowTimeBar->SetPercent(1.f);
		WindowTimeBar->SetFillColorAndOpacity(Entry.Color);
	}

	if ( IsValid(WindowPanel) )
	{
		WindowPanel->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	OnWindowShown(Entry);
}

float UYSHUDWindowRowWidget::GetNow() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetTimeSeconds() : 0.f;
}
