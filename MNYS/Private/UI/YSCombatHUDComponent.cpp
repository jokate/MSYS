// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/YSCombatHUDComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Character/AttributeSet/YSGroggyAttributeSet.h"
#include "Character/Components/YSCombatTargetComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "General/YSGameplayTag.h"
#include "Subsystem/YSSquadSubsystem.h"
#include "UI/YSGroggyGaugeWidget.h"
#include "UI/YSCombatHUDWidget.h"
#include "UI/YSHUDWindowRowWidget.h"
#include "UI/YSNearbyRadarWidget.h"
#include "UI/YSSwitchAttackToastWidget.h"
#include "UI/YSTargetMarkerWidget.h"

namespace
{
	UYSSquadSubsystem* GetSquad(const UWorld* World)
	{
		const UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
		return GameInstance ? GameInstance->GetSubsystem<UYSSquadSubsystem>() : nullptr;
	}
}

UYSCombatHUDComponent::UYSCombatHUDComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UYSCombatHUDComponent::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = Cast<APlayerController>(GetOwner());

	if ( IsValid(PC) == false || PC->IsLocalController() == false )
	{
		return;
	}

	if ( HUDWidgetClass == nullptr )
	{
		UE_LOG(LogTemp, Warning, TEXT("[CombatHUD] 전투 HUD 위젯 클래스가 비어 있어 HUD 를 띄우지 않았다. 컨트롤러 BP 에서 채워야 한다."));
		return;
	}

	HUDWidget = CreateWidget<UYSCombatHUDWidget>(PC, HUDWidgetClass);

	if ( IsValid(HUDWidget) == false )
	{
		return;
	}

	HUDWidget->AddToViewport();

	PC->OnPossessedPawnChanged.AddDynamic(this, &UYSCombatHUDComponent::HandlePossessedPawnChanged);
	BindPawnASC(PC->GetPawn());

	if ( UYSCombatTargetComponent* CombatTarget = UYSCombatTargetComponent::Get(PC) )
	{
		CombatTarget->OnTargetChanged.AddDynamic(this, &UYSCombatHUDComponent::HandleTargetChanged);
		HandleTargetChanged(nullptr, CombatTarget->GetCurrentTarget(), EYSTargetChangeReason::Auto);

		CombatTarget->OnNearbyEnemiesChanged.AddDynamic(this, &UYSCombatHUDComponent::HandleNearbyEnemiesChanged);
		HandleNearbyEnemiesChanged(CombatTarget->GetNearbyEnemies());
	}

	if ( UYSSquadSubsystem* Squad = GetSquad(GetWorld()) )
	{
		Squad->OnSwitchWindowOpened.AddDynamic(this, &UYSCombatHUDComponent::HandleSwitchWindowOpened);
		Squad->OnSwitchAttackFired.AddDynamic(this, &UYSCombatHUDComponent::HandleSwitchAttackFired);
	}
}

void UYSCombatHUDComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindPawnASC();
	UnbindTargetASC();

	if ( APlayerController* PC = Cast<APlayerController>(GetOwner()) )
	{
		PC->OnPossessedPawnChanged.RemoveDynamic(this, &UYSCombatHUDComponent::HandlePossessedPawnChanged);

		if ( UYSCombatTargetComponent* CombatTarget = UYSCombatTargetComponent::Get(PC) )
		{
			CombatTarget->OnTargetChanged.RemoveDynamic(this, &UYSCombatHUDComponent::HandleTargetChanged);
			CombatTarget->OnNearbyEnemiesChanged.RemoveDynamic(this, &UYSCombatHUDComponent::HandleNearbyEnemiesChanged);
		}
	}

	// 스쿼드는 게임 인스턴스 수명이라 컨트롤러보다 오래 산다. 레벨이 바뀌어도 죽은 구독이 남지 않게 끊는다.
	if ( UYSSquadSubsystem* Squad = GetSquad(GetWorld()) )
	{
		Squad->OnSwitchWindowOpened.RemoveDynamic(this, &UYSCombatHUDComponent::HandleSwitchWindowOpened);
		Squad->OnSwitchAttackFired.RemoveDynamic(this, &UYSCombatHUDComponent::HandleSwitchAttackFired);
	}

	if ( IsValid(HUDWidget) )
	{
		HUDWidget->RemoveFromParent();
		HUDWidget = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void UYSCombatHUDComponent::HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	UnbindPawnASC();
	BindPawnASC(NewPawn);
}

void UYSCombatHUDComponent::HandleTargetChanged(AActor* OldTarget, AActor* NewTarget, EYSTargetChangeReason Reason)
{
	if ( IsValid(HUDWidget) == false )
	{
		return;
	}

	if ( UYSTargetMarkerWidget* Marker = HUDWidget->GetTargetMarker() )
	{
		Marker->SetTarget(NewTarget);
	}

	if ( UYSNearbyRadarWidget* Radar = HUDWidget->GetNearbyRadar() )
	{
		Radar->SetCurrentTarget(NewTarget);
	}

	UnbindTargetASC();
	BindTargetASC(NewTarget);
	RefreshGauge();

	// 남용 쿨이 대상별인 정책이면 막대가 지금 타겟의 남은 시간을 따라가야 한다. 0 이면 숨는다.
	if ( UYSHUDWindowRowWidget* Row = HUDWidget->GetWindowRow() )
	{
		const UYSSquadSubsystem* Squad = GetSquad(GetWorld());
		Row->StartAbuseCooldown(Squad ? Squad->GetSwitchAbuseCooldownRemaining(NewTarget) : 0.f);
	}
}

void UYSCombatHUDComponent::HandleNearbyEnemiesChanged(const TArray<AActor*>& NearbyEnemies)
{
	if ( IsValid(HUDWidget) == false )
	{
		return;
	}

	if ( UYSNearbyRadarWidget* Radar = HUDWidget->GetNearbyRadar() )
	{
		Radar->SetEnemies(NearbyEnemies);
	}
}

void UYSCombatHUDComponent::HandleSwitchWindowOpened(EYSSwitchTrigger Trigger, AActor* Target, float AbuseCooldown)
{
	if ( IsValid(HUDWidget) == false )
	{
		return;
	}

	if ( UYSHUDWindowRowWidget* Row = HUDWidget->GetWindowRow() )
	{
		Row->StartAbuseCooldown(AbuseCooldown);
	}
}

void UYSCombatHUDComponent::HandleSwitchAttackFired(const FYSSwitchAttackReport& Report)
{
	if ( IsValid(HUDWidget) == false )
	{
		return;
	}

	if ( UYSSwitchAttackToastWidget* Toast = HUDWidget->GetSwitchAttackToast() )
	{
		Toast->ShowReport(Report);
	}
}

void UYSCombatHUDComponent::BindPawnASC(APawn* Pawn)
{
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn);

	if ( IsValid(ASC) == false )
	{
		return;
	}

	PawnASC = ASC;
	EffectAddedHandle = ASC->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(this, &UYSCombatHUDComponent::HandleEffectAdded);
	EffectRemovedHandle = ASC->OnAnyGameplayEffectRemovedDelegate().AddUObject(this, &UYSCombatHUDComponent::HandleEffectRemoved);
}

void UYSCombatHUDComponent::UnbindPawnASC()
{
	if ( UAbilitySystemComponent* ASC = PawnASC.Get() )
	{
		ASC->OnActiveGameplayEffectAddedDelegateToSelf.Remove(EffectAddedHandle);
		ASC->OnAnyGameplayEffectRemovedDelegate().Remove(EffectRemovedHandle);
	}

	PawnASC.Reset();
	EffectAddedHandle.Reset();
	EffectRemovedHandle.Reset();

	if ( IsValid(HUDWidget) )
	{
		if ( UYSHUDWindowRowWidget* Row = HUDWidget->GetWindowRow() )
		{
			Row->ClearWindows();
		}
	}
}

void UYSCombatHUDComponent::HandleEffectAdded(UAbilitySystemComponent* ASC, const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle Handle)
{
	if ( IsValid(HUDWidget) == false )
	{
		return;
	}

	if ( UYSHUDWindowRowWidget* Row = HUDWidget->GetWindowRow() )
	{
		Row->HandleEffectAdded(Spec, Handle);
	}
}

void UYSCombatHUDComponent::HandleEffectRemoved(const FActiveGameplayEffect& Effect)
{
	if ( IsValid(HUDWidget) == false )
	{
		return;
	}

	if ( UYSHUDWindowRowWidget* Row = HUDWidget->GetWindowRow() )
	{
		Row->HandleEffectRemoved(Effect.Handle);
	}
}

void UYSCombatHUDComponent::BindTargetASC(AActor* Target)
{
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);

	// 그로기 셋은 경직 등급 2 적에게만 붙는다(AYSMonster). 셋이 없으면 게이지를 보이지 않는다.
	if ( IsValid(ASC) == false || ASC->HasAttributeSetForAttribute(UYSGroggyAttributeSet::GetGroggyAttribute()) == false )
	{
		return;
	}

	TargetASC = ASC;
	GroggyChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(UYSGroggyAttributeSet::GetGroggyAttribute()).AddUObject(this, &UYSCombatHUDComponent::HandleGroggyChanged);
	MaxGroggyChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(UYSGroggyAttributeSet::GetMaxGroggyAttribute()).AddUObject(this, &UYSCombatHUDComponent::HandleGroggyChanged);
	GroggyBrokenHandle = ASC->GenericGameplayEventCallbacks.FindOrAdd(YSTags::Event_GroggyBroken).AddUObject(this, &UYSCombatHUDComponent::HandleGroggyBroken);

	// 구독 전에 이미 0 이었으면 Broken 은 다시 오지 않으므로 지금 값으로 정한다.
	bTargetBroken = ASC->GetNumericAttribute(UYSGroggyAttributeSet::GetGroggyAttribute()) <= 0.f;
}

void UYSCombatHUDComponent::UnbindTargetASC()
{
	if ( UAbilitySystemComponent* ASC = TargetASC.Get() )
	{
		ASC->GetGameplayAttributeValueChangeDelegate(UYSGroggyAttributeSet::GetGroggyAttribute()).Remove(GroggyChangedHandle);
		ASC->GetGameplayAttributeValueChangeDelegate(UYSGroggyAttributeSet::GetMaxGroggyAttribute()).Remove(MaxGroggyChangedHandle);

		if ( FGameplayEventMulticastDelegate* Broken = ASC->GenericGameplayEventCallbacks.Find(YSTags::Event_GroggyBroken) )
		{
			Broken->Remove(GroggyBrokenHandle);
		}
	}

	TargetASC.Reset();
	GroggyChangedHandle.Reset();
	MaxGroggyChangedHandle.Reset();
	GroggyBrokenHandle.Reset();
	bTargetBroken = false;
}

void UYSCombatHUDComponent::HandleGroggyChanged(const FOnAttributeChangeData& Data)
{
	const UAbilitySystemComponent* ASC = TargetASC.Get();

	if ( bTargetBroken && IsValid(ASC) && ASC->GetNumericAttribute(UYSGroggyAttributeSet::GetGroggyAttribute()) > 0.f )
	{
		bTargetBroken = false;
	}

	RefreshGauge();
}

void UYSCombatHUDComponent::HandleGroggyBroken(const FGameplayEventData* Payload)
{
	bTargetBroken = true;
	RefreshGauge();
}

void UYSCombatHUDComponent::RefreshGauge()
{
	if ( IsValid(HUDWidget) == false )
	{
		return;
	}

	UYSGroggyGaugeWidget* Gauge = HUDWidget->GetGroggyGauge();

	if ( IsValid(Gauge) == false )
	{
		return;
	}

	const UAbilitySystemComponent* ASC = TargetASC.Get();

	if ( IsValid(ASC) == false )
	{
		Gauge->SetGaugeState(EYSGroggyGaugeState::Hidden);
		return;
	}

	Gauge->SetGroggy(ASC->GetNumericAttribute(UYSGroggyAttributeSet::GetGroggyAttribute()), ASC->GetNumericAttribute(UYSGroggyAttributeSet::GetMaxGroggyAttribute()));
	Gauge->SetGaugeState(bTargetBroken ? EYSGroggyGaugeState::Broken : EYSGroggyGaugeState::Gauge);
}
