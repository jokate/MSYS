// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/YSNearbyRadarWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

UYSNearbyRadarWidget::UYSNearbyRadarWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 임시 모양. 그림이 없으면 색 사각형으로 그린다.
	PipBrush.ImageSize = FVector2D(10.f, 10.f);
	PipBrush.TintColor = FSlateColor(FLinearColor(1.f, 0.25f, 0.2f));

	TargetPipBrush.ImageSize = FVector2D(16.f, 16.f);
	TargetPipBrush.TintColor = FSlateColor(FLinearColor(1.f, 0.85f, 0.1f));
}

void UYSNearbyRadarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UYSNearbyRadarWidget::SetEnemies(const TArray<AActor*>& NewEnemies)
{
	Enemies.Reset(NewEnemies.Num());

	for ( AActor* Enemy : NewEnemies )
	{
		Enemies.Add(Enemy);
	}

	for ( int32 Index = 0; Index < Enemies.Num(); ++Index )
	{
		GetOrCreatePip(Index);
	}

	ApplyPipStyles();
}

void UYSNearbyRadarWidget::SetCurrentTarget(AActor* NewTarget)
{
	CurrentTarget = NewTarget;
	ApplyPipStyles();
}

UImage* UYSNearbyRadarWidget::GetOrCreatePip(int32 Index)
{
	if ( Pips.IsValidIndex(Index) )
	{
		return Pips[Index];
	}

	UImage* Pip = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());

	if ( UCanvasPanelSlot* PipSlot = PipCanvas->AddChildToCanvas(Pip) )
	{
		PipSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		PipSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		PipSlot->SetAutoSize(true);
	}

	Pips.Add(Pip);
	return Pip;
}

void UYSNearbyRadarWidget::ApplyPipStyles()
{
	for ( int32 Index = 0; Index < Pips.Num(); ++Index )
	{
		UImage* Pip = Pips[Index];

		if ( Enemies.IsValidIndex(Index) == false )
		{
			Pip->SetVisibility(ESlateVisibility::Collapsed);
			continue;
		}

		const bool bIsTarget = CurrentTarget.IsValid() && Enemies[Index] == CurrentTarget;
		Pip->SetBrush(bIsTarget ? TargetPipBrush : PipBrush);
	}
}

void UYSNearbyRadarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	const APlayerController* PC = GetOwningPlayer();
	const APawn* Pawn = IsValid(PC) ? PC->GetPawn() : nullptr;

	if ( IsValid(Pawn) == false )
	{
		return;
	}

	const float ViewYaw = PC->GetControlRotation().Yaw;

	for ( int32 Index = 0; Index < Enemies.Num() && Index < Pips.Num(); ++Index )
	{
		UImage* Pip = Pips[Index];
		const AActor* Enemy = Enemies[Index].Get();

		// 파괴된 적은 다음 목록이 올 때까지 숨긴다.
		if ( IsValid(Enemy) == false )
		{
			Pip->SetVisibility(ESlateVisibility::Collapsed);
			continue;
		}

		const FVector ToEnemy = Enemy->GetActorLocation() - Pawn->GetActorLocation();
		const float Angle = FRotator::NormalizeAxis(FMath::RadiansToDegrees(FMath::Atan2(ToEnemy.Y, ToEnemy.X)) - ViewYaw);
		const float Radians = FMath::DegreesToRadians(Angle);

		// 0도가 위(정면), 시계 방향이 오른쪽이다. 화면 Y 는 아래로 증가한다.
		if ( UCanvasPanelSlot* PipSlot = Cast<UCanvasPanelSlot>(Pip->Slot) )
		{
			PipSlot->SetPosition(FVector2D(FMath::Sin(Radians), -FMath::Cos(Radians)) * RadarRadius);
		}

		Pip->SetRenderTransformAngle(Angle);
		Pip->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}
