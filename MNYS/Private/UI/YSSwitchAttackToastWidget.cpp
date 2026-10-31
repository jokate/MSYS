// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/YSSwitchAttackToastWidget.h"

#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "TimerManager.h"

#define LOCTEXT_NAMESPACE "YSSwitchAttackToast"

void UYSSwitchAttackToastWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetVisibility(ESlateVisibility::Collapsed);
}

void UYSSwitchAttackToastWidget::NativeDestruct()
{
	if ( UWorld* World = GetWorld() )
	{
		World->GetTimerManager().ClearTimer(HideTimer);
	}

	Super::NativeDestruct();
}

void UYSSwitchAttackToastWidget::ShowReport(const FYSSwitchAttackReport& Report)
{
	if ( IsValid(RowText) )
	{
		RowText->SetText(FText::FromName(Report.Row));
	}

	if ( IsValid(GradeText) )
	{
		GradeText->SetText(( Report.Impact == EYSImpactGrade::None )
			? FText::GetEmpty()
			: StaticEnum<EYSImpactGrade>()->GetDisplayNameTextByValue(static_cast<int64>(Report.Impact)));
	}

	if ( IsValid(ScaleText) )
	{
		const bool bDecayed = Report.EffectScale < 1.f;

		if ( bDecayed )
		{
			FNumberFormattingOptions Format;
			Format.MaximumFractionalDigits = 2;
			ScaleText->SetText(FText::Format(LOCTEXT("Scale", "감쇠 ×{0}"), FText::AsNumber(Report.EffectScale, &Format)));
		}

		ScaleText->SetVisibility(bDecayed ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}

	if ( IsValid(FirstGrabMark) )
	{
		FirstGrabMark->SetVisibility(Report.bFirstGrab ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}

	SetVisibility(ESlateVisibility::HitTestInvisible);
	OnToastShown(Report);

	UWorld* World = GetWorld();

	if ( World == nullptr || DisplayDuration <= 0.f )
	{
		return;
	}

	World->GetTimerManager().SetTimer(HideTimer, FTimerDelegate::CreateUObject(this, &UYSSwitchAttackToastWidget::HideToast), DisplayDuration, false);
}

void UYSSwitchAttackToastWidget::HideToast()
{
	SetVisibility(ESlateVisibility::Collapsed);
	OnToastHidden();
}

#undef LOCTEXT_NAMESPACE
