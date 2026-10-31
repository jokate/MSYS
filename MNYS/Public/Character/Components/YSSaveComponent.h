// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "General/YSStruct.h"
#include "YSSaveComponent.generated.h"


/* 기사단장 혼이 사용할 컴포넌트. 세이브를 이용한 공격을 전제로 한다. */

class AYSSaveEcho;
struct FGameplayTag;
struct FYSAbilityHitContext;
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

USTRUCT(BlueprintType)
struct FYSSavedTechniqueSlot
{
	GENERATED_BODY();

public : 
	UPROPERTY(VisibleAnywhere, Category = "YS | Save", meta = (DisplayName = "저장된 동작"))
	FYSSavedTechnique SavedTechnique;

	UPROPERTY(VisibleAnywhere, Category = "YS | Save", meta = (DisplayName = "슬롯 인덱스"))
	float LastUsedTime = 0.f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSaveStateChanged);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MNYS_API UYSSaveComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	static UYSSaveComponent* Get(AActor* Owner);
	UYSSaveComponent();

	void MarkSavable(TSubclassOf<UYSGameplayAbility> AbilityClass, int32 PlaybackIndex, float Multiplier);
	
	/* 세이브 입력. Pending을 슬롯으로 올린다. 올릴 게 없거나 슬롯이 꽉 차면 실패. */
	bool TryCommitPending();

	/** 배치 시 호출. 가장 오래된 슬롯 1건을 꺼내 비운다. */
	bool TryConsumeSlot(FYSSavedTechnique& OutTechnique);

	/** 분신이 스폰될 때 부른다. 같은 히트 기록으로 대기 중인 삼백의 참격 기술이 있으면 그것을, 없으면 슬롯을 꺼낸다. */
	bool TakeEchoTechnique(const TSharedPtr<FYSAbilityHitContext>& HitContext, FYSSavedTechnique& OutTechnique);

	/** 기술을 대기열에 넣고 분신을 스폰한다. 낸 어빌리티나 궁극기가 스폰 전에 끝나면 그 기술은 버려진다. */
	void ExecuteTripleEcho(const FYSSavedTechnique& Technique, const TSharedPtr<FYSAbilityHitContext>& HitContext);
	
protected:
	virtual void BeginPlay() override;
	
	
	bool HasSavedTechnique() const;
	bool IsSlotFull() const;
	bool HasPending() const { return PendingTechnique.IsValid(); }

	int32 GetSavedCount() const { return SavedTechniques.Num(); }
	const FYSSavedTechnique& GetPendingTechnique() const { return PendingTechnique; }
	
	void SetMaxSlotCount(int32 NewMax);
	
protected:
	UFUNCTION()
	void HandleRawInput(const FGameplayTag& InputTag, EYSInputPhase InputPhase);

	UFUNCTION()
	void HandleTagStateChanged(const FGameplayTag& Tag, bool bTagExists);

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

	/** 궁극기가 끝나면 예약분은 나가지 않는다. 버프 밖에서 참격이 나오면 안 된다. */
	void CancelPendingEchoes();

protected:
	// 혼의 세이브는 컨셉상 2개 2개만 저장해야 한다./
	UPROPERTY(VisibleAnywhere, Category = "YS | Save", meta = (DisplayName = "저장된 동작들"))
	TArray<FYSSavedTechniqueSlot> SavedTechniques;
	
	UPROPERTY(VisibleAnywhere, Category = "YS | Save", meta = (DisplayName = "저장 가능한 직전 동작"))
	FYSSavedTechnique PendingTechnique;

	UPROPERTY(EditDefaultsOnly, Category = "YS | Save", meta = (DisplayName = "세이브 쿨타임"))
	float SaveCooldown = 2.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "YS | Save", meta = (DisplayName = "최대 슬롯 수"))
	int32 MaxSlotCount = 2;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (DisplayName = "삼백의 참격 분신 스폰 설정"))
	FYSSpawnActorConfig TripleEchoSpawnConfig;
	
private :
	// 스폰을 기다리는 삼백의 참격 기술과 그것을 낸 어빌리티의 히트 기록. 요청 순서대로 나온다.
	TArray<TPair<FYSSavedTechnique, TWeakPtr<FYSAbilityHitContext>>> QueuedEchoTechniques;
	
};
