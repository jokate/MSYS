// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/YSTargetMarkerWidget.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"

void UYSTargetMarkerWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 자신을 접으면 틱이 멈춰 다시 켤 수 없다. 접는 것은 꺾쇠만 한다.
	SetVisibility(ESlateVisibility::HitTestInvisible);
	HideMarker();
}

void UYSTargetMarkerWidget::SetTarget(AActor* NewTarget)
{
	const UWorld* World = GetWorld();

	if ( IsValid(NewTarget) )
	{
		Target = NewTarget;
		LostTime = -1.f;
		SetRenderOpacity(1.f);
		return;
	}

	if ( Target.IsValid() && World != nullptr && LostFadeDuration > 0.f )
	{
		LostTime = World->GetTimeSeconds();
		SetRenderOpacity(LostOpacity);
		return;
	}

	HideMarker();
}

void UYSTargetMarkerWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if ( Target.IsValid() == false )
	{
		if ( LeftBracket->GetVisibility() != ESlateVisibility::Collapsed )
		{
			HideMarker();
		}
		return;
	}

	const UWorld* World = GetWorld();

	if ( LostTime >= 0.f && World != nullptr && World->GetTimeSeconds() - LostTime >= LostFadeDuration )
	{
		HideMarker();
		return;
	}

	FVector2D ScreenMin;
	FVector2D ScreenMax;

	if ( GetScreenRect(Target.Get(), ScreenMin, ScreenMax) == false )
	{
		SetBracketsVisible(false);
		return;
	}

	PlaceBrackets(ScreenMin, ScreenMax);
	SetBracketsVisible(true);
}

bool UYSTargetMarkerWidget::GetScreenRect(const AActor* Actor, FVector2D& OutMin, FVector2D& OutMax) const
{
	const APlayerController* PC = GetOwningPlayer();

	if ( IsValid(PC) == false )
	{
		return false;
	}

	FVector Origin;
	FVector Extent;
	Actor->GetActorBounds(true, Origin, Extent);

	OutMin = FVector2D(TNumericLimits<float>::Max());
	OutMax = FVector2D(TNumericLimits<float>::Lowest());

	for ( int32 Corner = 0; Corner < 8; ++Corner )
	{
		const FVector Sign((Corner & 1) ? 1.f : -1.f, (Corner & 2) ? 1.f : -1.f, (Corner & 4) ? 1.f : -1.f);
		FVector2D ScreenPos;

		// 꼭짓점 하나라도 카메라 뒤면 영역이 뒤집혀 보이므로 표지를 숨긴다.
		if ( UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, Origin + Extent * Sign, ScreenPos, false) == false )
		{
			return false;
		}

		OutMin = FVector2D::Min(OutMin, ScreenPos);
		OutMax = FVector2D::Max(OutMax, ScreenPos);
	}

	return true;
}

void UYSTargetMarkerWidget::PlaceBrackets(const FVector2D& ScreenMin, const FVector2D& ScreenMax)
{
	const float Height = ScreenMax.Y - ScreenMin.Y;

	if ( UCanvasPanelSlot* LeftSlot = Cast<UCanvasPanelSlot>(LeftBracket->Slot) )
	{
		const float Width = LeftSlot->GetSize().X;
		LeftSlot->SetPosition(FVector2D(ScreenMin.X - BracketGap - Width, ScreenMin.Y));
		LeftSlot->SetSize(FVector2D(Width, Height));
	}

	if ( UCanvasPanelSlot* RightSlot = Cast<UCanvasPanelSlot>(RightBracket->Slot) )
	{
		RightSlot->SetPosition(FVector2D(ScreenMax.X + BracketGap, ScreenMin.Y));
		RightSlot->SetSize(FVector2D(RightSlot->GetSize().X, Height));
	}
}

void UYSTargetMarkerWidget::HideMarker()
{
	Target.Reset();
	LostTime = -1.f;
	SetRenderOpacity(1.f);
	SetBracketsVisible(false);
}

void UYSTargetMarkerWidget::SetBracketsVisible(bool bVisible)
{
	const ESlateVisibility BracketVisibility = bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed;
	LeftBracket->SetVisibility(BracketVisibility);
	RightBracket->SetVisibility(BracketVisibility);
}
