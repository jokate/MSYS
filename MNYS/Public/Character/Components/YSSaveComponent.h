// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "YSSaveComponent.generated.h"


/*
 * 기사단장 혼이 사용할 컴포넌트
 * 혼은 세이브를 이용한 공격을 전제로 합니다.
 */

class UYSAbilitySystemComponent;
class UYSGameplayAbility;

USTRUCT(BlueprintType)
struct FYSSavedTechnique
{
	GENERATED_BODY()
	
public :
	FYSSavedTechnique() = default;
	FYSSavedTechnique(TSubclassOf<UYSGameplayAbility> InAbilityClass, int32 InPlaybackIndex, float InMultiplier)
		: AbilityClass(InAbilityClass), PlaybackIndex(InPlaybackIndex), SavedMultiplier(InMultiplier)
	{
	}
	
	UPROPERTY(VisibleAnywhere, Category = "YS | Save", meta = (DisplayName = "저장된 어빌리티"))
	TSubclassOf<UYSGameplayAbility> AbilityClass;

	// 어빌리티 트리거 할 건데 어떤 세이브를 발동 시킬 것인가?
	UPROPERTY(VisibleAnywhere, Category = "YS | Save", meta = (DisplayName = "저장된 플레이백 노드"))
	int32 PlaybackIndex = INDEX_NONE;

	// 배치에 따른 배수 공격 자체를 한다.
	UPROPERTY(VisibleAnywhere, Category = "YS | Save", meta = (DisplayName = "저장 시점 배율"))
	float SavedMultiplier = 2.f;

	bool IsValid() const;
	void Reset();
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSaveStateChanged);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MNYS_API UYSSaveComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	static UYSSaveComponent* Get(AActor* Owner);
	// Sets default values for this component's properties
	UYSSaveComponent();

	void MarkSavable(TSubclassOf<UYSGameplayAbility> AbilityClass, int32 PlaybackIndex, float Multiplier);
	
	/* 세이브 입력. Pending을 슬롯으로 올린다. 올릴 게 없거나 슬롯이 꽉 차면 실패. */
	bool TryCommitPending();

	/** 배치 시 호출. 가장 오래된 슬롯 1건을 꺼내 비운다. */
	bool TryConsumeSlot(FYSSavedTechnique& OutTechnique);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	
	bool HasSavedTechnique() const { return SavedTechniques.Num() > 0; }
	bool IsSlotFull() const { return SavedTechniques.Num() >= MaxSlotCount; }
	bool HasPending() const { return PendingTechnique.IsValid(); }

	int32 GetSavedCount() const { return SavedTechniques.Num(); }
	const FYSSavedTechnique& GetPendingTechnique() const { return PendingTechnique; }
	
	void SetMaxSlotCount(int32 NewMax);
	
public:
	UPROPERTY(BlueprintAssignable, Category = "YS | Save", meta = (DisplayName = "세이브 상태 변경"))
	FOnSaveStateChanged OnSaveStateChanged;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Pending 만료. 타이머 콜백. */
	UFUNCTION()
	void ClearPending();
	
	void RefreshStateTags();

	UYSAbilitySystemComponent* GetOwnerASC() const;
	
protected:
	// 혼의 세이브는 컨셉상 2개 2개만 저장해야 한다./
	UPROPERTY(VisibleAnywhere, Category = "YS | Save", meta = (DisplayName = "저장된 동작들"))
	TArray<FYSSavedTechnique> SavedTechniques;
	
	UPROPERTY(VisibleAnywhere, Category = "YS | Save", meta = (DisplayName = "저장 가능한 직전 동작"))
	FYSSavedTechnique PendingTechnique;

	UPROPERTY(EditDefaultsOnly, Category = "YS | Save", meta = (DisplayName = "최대 슬롯 수"))
	int32 MaxSlotCount = 2;
};
