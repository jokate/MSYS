// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "Components/ActorComponent.h"
#include "General/YSEnum.h"
#include "Subsystem/YSSquadSubsystem.h"
#include "YSCombatHUDComponent.generated.h"

class UAbilitySystemComponent;
class UYSCombatHUDWidget;
struct FActiveGameplayEffect;
struct FGameplayEffectSpec;
struct FGameplayEventData;
struct FOnAttributeChangeData;

/** 전투 HUD. 이벤트 구독을 전부 여기 한 곳에 둔다. 컨트롤러에 두는 것은 타겟 컴포넌트와 같은 이유로, 폰이 바뀌어도 HUD 가 이어져야 해서다. 폰 교대 · 타겟 변경 때 구독할 ASC 를 갈아끼우고 받은 값을 위젯에 내리기만 하며 폴링하지 않는다. */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MNYS_API UYSCombatHUDComponent : public UActorComponent
{
	GENERATED_BODY()

public :
	UYSCombatHUDComponent();

	UFUNCTION(BlueprintPure, Category = "YS | HUD")
	UYSCombatHUDWidget* GetHUDWidget() const { return HUDWidget; }

protected :
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private :
	UFUNCTION()
	void HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);

	UFUNCTION()
	void HandleTargetChanged(AActor* OldTarget, AActor* NewTarget, EYSTargetChangeReason Reason);

	UFUNCTION()
	void HandleNearbyEnemiesChanged(const TArray<AActor*>& NearbyEnemies);

	UFUNCTION()
	void HandleSwitchWindowOpened(EYSSwitchTrigger Trigger, AActor* Target, float AbuseCooldown);

	UFUNCTION()
	void HandleSwitchAttackFired(const FYSSwitchAttackReport& Report);

	// 플레이어 쪽: 시간창 GE
	void BindPawnASC(APawn* Pawn);
	void UnbindPawnASC();
	void HandleEffectAdded(UAbilitySystemComponent* ASC, const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle Handle);
	void HandleEffectRemoved(const FActiveGameplayEffect& Effect);

	// 타겟 쪽: 그로기 게이지
	void BindTargetASC(AActor* Target);
	void UnbindTargetASC();
	void HandleGroggyChanged(const FOnAttributeChangeData& Data);
	void HandleGroggyBroken(const FGameplayEventData* Payload);
	void RefreshGauge();

private :
	UPROPERTY(EditDefaultsOnly, Category = "YS | HUD", meta = (DisplayName = "전투 HUD 위젯"))
	TSubclassOf<UYSCombatHUDWidget> HUDWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UYSCombatHUDWidget> HUDWidget;

	TWeakObjectPtr<UAbilitySystemComponent> PawnASC;
	FDelegateHandle EffectAddedHandle;
	FDelegateHandle EffectRemovedHandle;

	TWeakObjectPtr<UAbilitySystemComponent> TargetASC;
	FDelegateHandle GroggyChangedHandle;
	FDelegateHandle MaxGroggyChangedHandle;
	FDelegateHandle GroggyBrokenHandle;

	// Broken 이벤트로 들어가고, 그로기 게이지가 0 위로 돌아오면 나온다.
	bool bTargetBroken = false;
};
