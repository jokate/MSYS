// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "General/YSStruct.h"
#include "Ability/AbilityComponent/YSPlaybackCondition.h"
#include "StructUtils/InstancedStruct.h"
#include "UObject/Object.h"
#include "YSAbilityPlayback.generated.h"

class ULevelSequencePlayer;
class UAbilityTask_PlayMontageAndWait;
class ALevelSequenceActor;
/**
 * 
 */
UENUM(BlueprintType)
enum class EYSAbilityPlaybackType : uint8
{
	None UMETA(DisplayName = "아무것도 재생 안함"),
	Montage UMETA(DisplayName = "몽타주 재생"),
	Sequence UMETA(DisplayName = "시퀀스 재생"),
};

USTRUCT()
struct FYSPlaybackEdge
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "YS | Transition", meta = (DisplayName = "발화 조건"))
	EYSPlaybackEvent RequiredResult = EYSPlaybackEvent::Completed;
	
	UPROPERTY(EditDefaultsOnly, Category = "YS | Transition", meta = (DisplayName = "전환 조건", BaseStruct = "/Script/MNYS.YSPlaybackCondition", ExcludeBaseStruct))
	TArray<FInstancedStruct> TransitionConditions;
	
	// -1 = 체인 종료 (어빌리티 EndAbility)
	UPROPERTY(EditDefaultsOnly, Category = "YS | Transition", meta = (DisplayName = "다음 노드 인덱스 (-1: 종료)"))
	int32 NextNodeIndex = INDEX_NONE;
};

// 해당 구조의 가장 큰 문제점은 어빌리티의 플레이 백을 의미하다보니 다른 어빌리티에서 동작 시, Race Condition이 발생할 수 있음.
// 사실 이게 다양한 상황에서의 전제가 있다고 가정한다면, 애니메이션이 좀 꼬일 수 있겠다는 생각은 드는 편.
// 그렇다면 규칙이 있음, 예를 들어서 

UCLASS(EditInlineNew, DefaultToInstanced, CollapseCategories)
class MNYS_API UYSAbilityPlaybackBase : public UObject
{
	GENERATED_BODY()
	
public : 
	void Play(const FYSPlaybackContext& Context);
	void ReleaseMotionWarp();
	bool DispatchNext(EYSPlaybackEvent Event, bool bIsEvaluate = false);
	void OnHit(const TArray<FHitResult>& HitResults);
	
	const FYSPlaybackContext& GetCurrentPlaybackContext() const { return CapturedContext; }
	AActor* GetCurrentPlaybackTarget() const { return CapturedContext.Target; }
	virtual void EndPlay();
	
	bool TryAcceptContextTag(const FGameplayTag& InputTag);
	
protected :
	UFUNCTION()
	void OnMontagePlayed();

	UFUNCTION()
	void OnMontageInterrupted();
	
	virtual void SetupMontage(const FYSPlaybackContext& Context);
	
	virtual void SetPlayback(const FYSPlaybackContext& Context);
	
	UFUNCTION()
	void OnSequencePlayed();
	
	virtual void SetupSequence(const FYSPlaybackContext& Context);

	virtual void ProcessContextBeforePlay() {};
	
public : 
	UPROPERTY(EditDefaultsOnly, Category = "YS | Ability Playback Type")
	EYSAbilityPlaybackType PlaybackType = EYSAbilityPlaybackType::Montage;
	
	UPROPERTY(EditDefaultsOnly, Category = "YS | Sequence", meta = (DisplayName = "시퀀스 설정", 	EditCondition = "PlaybackType == EYSAbilityPlaybackType::Sequence"))
	FYSSequencePlaySettings SequenceSettings;
	
	UPROPERTY(EditDefaultsOnly, Category = "YS | Montage To Play", meta = (DisplayName = "재생할 몽타주 정보", EditCondition = "PlaybackType == EYSAbilityPlaybackType::Montage", BaseStruct = "/Script/MNYS.YSMontageSelector", ExcludeBaseStruct))
	FInstancedStruct MontageSelector;

	UPROPERTY(EditDefaultsOnly, Category = "YS | Transitions", meta = (DisplayName = "다음 플레이백 전환"))
	TArray<FYSPlaybackEdge> Transitions;
	
	UPROPERTY(EditDefaultsOnly, Category = "YS | Transitions", meta = (DisplayName = "즉시 전환 가능 여부"))
	bool bImmediateTransition = false;

protected : 
	// Play() 시점에 캡처 — 콜백에서 컨텍스트 참조용
	FYSPlaybackContext CapturedContext;
private :
	UPROPERTY()
	TObjectPtr<ALevelSequenceActor> ActiveSequenceActor = nullptr;
	
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> PlayMontageAndWaitTask = nullptr;
	
	UPROPERTY()
	TObjectPtr<ULevelSequencePlayer> LevelSequencePlayer = nullptr;
	
};

UCLASS(DisplayName = "(플레이어 한정) 락온 된 대상 타겟기준 플레이 백")
class MNYS_API UYSAbilityPlayback_LockonTarget : public UYSAbilityPlaybackBase
{
	GENERATED_BODY()

protected : 
	virtual void ProcessContextBeforePlay() override;
};

UCLASS(DisplayName = "히트 첫번째 타겟 기준 플레이 백")
class MNYS_API UYSAbilityPlayback_FirstHitTarget : public UYSAbilityPlaybackBase
{
	GENERATED_BODY()
	
protected :
	virtual void ProcessContextBeforePlay() override;
};

